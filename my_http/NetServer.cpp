#include "NetServer.h"
//#include "NewRequest.h"
#include "NewResponse.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "Timer.h"
//#include "SocketsHel.h"
#include "ComAddress.h"
//#include "Utils.h"


#include <iostream>
#include <functional> 
#include <cstring> //bzero
#include <cassert> //assert

#include <unistd.h>  //close,read
#include <sys/socket.h> //socket相关
#include <arpa/inet.h> //sockaddr_in相关


//using std::placeholders::_1;

//const int NetServer::TIMEOUTMS_ = -1;
//const int NetServer::CONNECT_TIMEOUT_ = 500;
//const int NetServer::poolSize_ = 4;

NetServer::NetServer(const ComAddress& serverAddr,int numThread)
	:port_(serverAddr.dport_stctoin()),
	 listenFd_(createNonblockingHel(port_)),
	 serverRequest_(new NewRequest(listenFd_)),
	 serverSocket_(listenFd_),
	 epoll_(new Epoll()),
	 threadPool_(new ThreadPool(numThread)),
	 timerTable_(new TimerTable())
	{
	 	assert(listenFd_ >= 0);
		serverSocket_.setReuseAddr(true);
		//serverSocket_.setReusePort(true);
		/*printf("NetServer_fd=%d\n",listenFd_);
		struct sockaddr_in addr;
		::bzero((char*)&addr,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons((unsigned short)port_);
		addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
		//::inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
		/*if(::bind(listenFd_,sockaddr_cast(&addr),static_cast<socklen_t>(sizeof(addr)))==-1){
			printf("SocketsHel::createNonblockingHel fd=%d bind:%s\n",listenFd_,strerror(errno));
		}*/
		//bindHel(listenFd_,sockaddr_cast(&addr));
		serverSocket_.bindAddr(serverAddr);
		//inet_ntop(AF_INET,&addr_.sin_addr.s_addr,buf,64);
		listenHel(listenFd_);		
		
	 }

 

	
NetServer::~NetServer()
{}

//开启服务器
/*
void NetServer::recvConnection()
{
	while(1) {
		int acceptFd = ::accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
		printf("acceptFd=%d\n",acceptFd);
		if(acceptFd == -1) {
			if(errno == EAGAIN)
				break;
			printf("[NetServer::recvConnection] accept : %s\n", strerror(errno));
			break;
		}
		//为新的连接套接字分配NewRequest资源
		NewRequest* requPtrest = new NewRequest(acceptFd);
		timerTable_ -> addTimer(requPtrest, CONNECT_TIMEOUT, std::bind(&NetServer::shutDownConnection, this, requPtrest));
		// 注册连接套接字到epoll（可读，边缘触发，保证任一时刻只被一个线程处理）
		epoll_ -> add(acceptFd, requPtrest, (EPOLLIN | EPOLLONESHOT));
	}
}
*/
void NetServer::start(){
	//serverSocket_.startListen();

	//添加服务器监听套接字到epoll下,并注册可读事件
	epoll_ -> add(listenFd_,serverRequest_,(EPOLLIN | EPOLLET));
	//注册新建连接回调事件	
	epoll_ -> setNewRequestCallback(std::bind(&NetServer::recvConnection,this));
	//注册关闭连接回调函数
	epoll_ -> setCloseRequestCallback(std::bind(&NetServer::shutDownConnection,this,std::placeholders::_1));

	//注册请求处理回调函数
	epoll_ -> setHandleRequestCallback(std::bind(&NetServer::handleRequest,this,std::placeholders::_1));
	//注册响应处理回调函数
	epoll_ -> setHandleResponseCallback(std::bind(&NetServer::handleResponse,this,std::placeholders::_1));

	//开始监测事件循环
	//printf("监听端口为=%d,listenFd_=%d\n",port_,listenFd_);
	while(1){
		int timeMS = timerTable_ -> getNextDeadTimer();
		//printf("开始监听listenFd=%d,持续时长为=%d\n",listenFd_,timeMS);
		int positEvents = epoll_ -> wait(timeMS);
		//printf("当前活跃fd个数=%d\n",positEvents);
		if(positEvents > 0){
			//处理活跃事件
			epoll_ -> handleEvent(listenFd_,threadPool_,positEvents);
		}
		//printf("整理timerTable\n");
		timerTable_ -> manageDeadTimers();
	}
}

//ET模式
void NetServer::recvConnection(){
	//因为listenFd_使用的是ET模式且一次可能有多个连接一同触发一个新连接请求,所以要在一次触发后,使用while循环不断读取并给他们分配
	//newFd,只有这样的话才能尽量不漏掉所有的客户端连接.
	while(1){
		int newFd = ::accept4(listenFd_,nullptr,nullptr,SOCK_NONBLOCK | SOCK_CLOEXEC);
		//printf("接收到新的连接,创建新的fd=%d\n",newFd);
		if(newFd == -1){
			if(errno == EAGAIN)
				break;
			//printf("NetServer::recvConnection accept : %s\n",strerror(errno));
			break;
		}
		//为新的连接创建实体类,并分配资源
		NewRequestPtr requPtr(new NewRequest(newFd));
		//放入容器,保持NewRequest资源不被自动清除
		ptrsMap_[newFd] = requPtr;
		timerTable_ -> addTimer(requPtr,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,newFd));
		//注册套接字到epoll下监测设定的事件
		epoll_ -> add(newFd,requPtr,(EPOLLIN | EPOLLONESHOT));
	}

}

void NetServer::shutDownConnection(int newFd){
	//printf("NetServer::shutDownConnection__fd=%d\n",newFd);
	int fd = newFd;
	{
		//printf("pre+++++++\n");
		NewRequestPtr requPtr = ptrsMap_[newFd];
		//printf("tail++++++requ的引用为=%d\n",requPtr.use_count());
		if(requPtr -> isWorking()){
			return;
		}

		timerTable_->delTimer(requPtr);
		epoll_ -> del(fd,requPtr,0);
	}
	ptrsMap_.erase(fd);
}


void NetServer::handleRequest(int newFd){
	NewRequestPtr requPtr = ptrsMap_[newFd];
	//printf("NetServer::handleRequest__fd=%d\n",newFd);
	/*
	 * 这里delTimer后,如果读取数据成功,那么为这个fd添加Timer的操作是在handleResponse里面.
	 */
	timerTable_ -> delTimer(requPtr);
	//assert(requPtr != nullptr);
	int fd = newFd;
	int readErrno;
	//printf("读取的fd=%d\n",fd);
	int nRead = requPtr -> read(&readErrno);

	//表示客户端已断开连接
	if(nRead == 0){
		//printf("nRead=%d\n",nRead);
		requPtr -> setNoWorking();
		shutDownConnection(fd);
		return;
	}
	//非EAGIN错误,断开连接
	if(nRead < 0 && (readErrno != EAGAIN)){
		//printf("!EAGAIN-nRead=%d\n",nRead);
		requPtr -> setNoWorking();
		shutDownConnection(fd);
		return;
	}	
	//EAGAIN错误则释放线程使用权，，并监听下次EPOLLIN事件
	if(nRead < 0 && readErrno == EAGAIN){
		//printf("EAGAIN-nRead=%d\n",nRead);

		epoll_ -> mod(fd,requPtr,(EPOLLIN | EPOLLONESHOT));
		requPtr -> setNoWorking();
		timerTable_ -> addTimer(requPtr,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,fd));
		return;
	}
	//如果解析报文出错，，就构造400报文返回
	if(!requPtr -> parseRequest()){
		//printf("!parseRequest\n");

		//发送400报文
		NewResponse respon(400,"",false);
		requPtr -> appendToOutBuffer(respon.makeResponse());
		//这里不用触发EPOLLOUT事件,直接发送400报文,然后立刻关闭连接
		int writeErrno;
		requPtr -> write(&writeErrno);
		requPtr-> setNoWorking();
		shutDownConnection(fd);
		return;
	}
	//如果解析成功,那么就发送200报文,触发EPOLLOUT事件发送;
	if(requPtr -> parseFinish()){
		//printf("parseFinish\n");
		NewResponse respon(200,requPtr -> getPath(),requPtr -> keepAlive());
		requPtr -> appendToOutBuffer(respon.makeResponse());
		epoll_->mod(fd,requPtr,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));	
	}
}	
//处理响应请求
void NetServer::handleResponse(int newFd){
	NewRequestPtr requPtr = ptrsMap_[newFd];
	//printf("NetServer::handleResponse__fd=%d__requ的引用数为%d\n",newFd,requPtr.use_count());
	timerTable_->delTimer(requPtr);

	assert(requPtr.use_count() >0);
	int fd = newFd;

	int toSend = requPtr -> writableBytes();
	if(toSend == 0){
		epoll_-> mod(fd,requPtr,(EPOLLIN | EPOLLONESHOT));
		requPtr -> setNoWorking();
		timerTable_ -> addTimer(requPtr,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,fd));
		return;
	}
	
	//此时outBuffer中有要发送的
	int writeErrno;
	int ret = requPtr -> write(&writeErrno);
	
	//非EAGAIN错误,则断开连接
	if(ret < 0 && (writeErrno != EAGAIN)){
		requPtr -> setNoWorking();
		shutDownConnection(fd);
		return;
	}

	if(ret < 0 && writeErrno == EAGAIN){
		epoll_-> mod(fd,requPtr,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));
		return;
		
	}

	if(ret == toSend){
		if(requPtr -> keepAlive()){
			requPtr -> resetParse();
			epoll_ -> mod(fd,requPtr,(EPOLLIN | EPOLLONESHOT));
			requPtr -> setNoWorking();
			timerTable_ -> addTimer(requPtr,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,fd));
		}else{
			requPtr -> setNoWorking();
			shutDownConnection(fd);
		}
		return;
	}

	epoll_ -> mod(fd,requPtr,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));
	requPtr -> setNoWorking();
	timerTable_ -> addTimer(requPtr,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,fd));
	return;
}



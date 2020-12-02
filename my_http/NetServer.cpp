#include "NetServer.h"
#include "NewRequest.h"
#include "NewResponse.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "SocketsHel.h"
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

NetServer::NetServer(int port,int numThread)
	:port_(port),
	 listenFd_(createNonblockingHel(port_)),
	 serverRequest_(new NewRequest(listenFd_)),
	 epoll_(new Epoll()),
	 threadPool_(new ThreadPool(numThread)),
	 timerTable_(new TimerTable()){
	 	assert(listenFd_ >= 0);
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
		NewRequest* request = new NewRequest(acceptFd);
		timerTable_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::shutDownConnection, this, request));
		// 注册连接套接字到epoll（可读，边缘触发，保证任一时刻只被一个线程处理）
		epoll_ -> add(acceptFd, request, (EPOLLIN | EPOLLONESHOT));
	}
}
*/
void NetServer::start(){
	//添加服务器监听套接字到epoll下,并注册可读事件
	epoll_ -> add(listenFd_,serverRequest_.get(),(EPOLLIN | EPOLLET));
	//注册新建连接回调事件	
	epoll_ -> setNewRequestCallback(std::bind(&NetServer::recvConnection,this));
	//注册关闭连接回调函数
	epoll_ -> setCloseRequestCallback(std::bind(&NetServer::shutDownConnection,this,std::placeholders::_1));

	//注册请求处理回调函数
	epoll_ -> setHandleRequestCallback(std::bind(&NetServer::handleRequest,this,std::placeholders::_1));
	//注册响应处理回调函数
	epoll_ -> setHandleResponseCallback(std::bind(&NetServer::handleResponse,this,std::placeholders::_1));

	//开始监测事件循环
	printf("监听端口为=%d,listenFd_=%d\n",port_,listenFd_);
	while(1){
		int timeMS = timerTable_ -> getNextDeadTimer();
		printf("开始监听listenFd=%d,持续时长为=%d\n",listenFd_,timeMS);
		int positEvents = epoll_ -> wait(timeMS);
		printf("当前活跃fd个数=%d\n",positEvents);
		if(positEvents > 0){
			//处理活跃事件
			epoll_ -> handleEvent(listenFd_,threadPool_,positEvents);
		}
		printf("整理timerTable\n");
		timerTable_ -> manageDeadTimers();
	}
}

//ET模式
void NetServer::recvConnection(){
	//因为listenFd_使用的是ET模式且一次可能有多个连接一同触发一个新连接请求,所以要在一次触发后,使用while循环不断读取并给他们分配
	//newFd,只有这样的话才能尽量不漏掉所有的客户端连接.
	while(1){
		int newFd = ::accept4(listenFd_,nullptr,nullptr,SOCK_NONBLOCK | SOCK_CLOEXEC);
		printf("接收到新的连接,创建新的fd=%d\n",newFd);
		if(newFd == -1){
			if(errno == EAGAIN)
				break;
			//printf("NetServer::recvConnection accept : %s\n",strerror(errno));
			break;
		}
		//为新的连接创建实体类,并分配资源
		NewRequest* requ = new NewRequest(newFd);
		timerTable_ -> addTimer(requ,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,requ));
		//注册套接字到epoll下监测设定的事件
		epoll_ -> add(newFd,requ,(EPOLLIN | EPOLLONESHOT));
	}

}

void NetServer::shutDownConnection(NewRequest* requ){
	printf("NetServer::shutDownConnection\n");
	int fd = requ -> fd();
	if(requ -> isWorking()){
			return;
	}

	timerTable_->delTimer(requ);
	epoll_ -> del(fd,requ,0);
	delete requ;
	requ=nullptr;
}


void NetServer::handleRequest(NewRequest* requ){
	printf("NetServer::handleRequest\n");
	timerTable_ -> delTimer(requ);
	assert(requ != nullptr);
	int fd = requ -> fd();
	int readErrno;
	printf("读取的fd=%d\n",fd);
	int nRead = requ -> read(&readErrno);

	//表示客户端已断开连接
	if(nRead == 0){
		//printf("nRead=%d\n",nRead);
		requ -> setNoWorking();
		shutDownConnection(requ);
		return;
	}
	//非EAGIN错误,断开连接
	if(nRead < 0 && (readErrno != EAGAIN)){
		//printf("!EAGAIN-nRead=%d\n",nRead);
		requ -> setNoWorking();
		shutDownConnection(requ);
		return;
	}	
	//EAGAIN错误则释放线程使用权，，并监听下次EPOLLIN事件
	if(nRead < 0 && readErrno == EAGAIN){
		//printf("EAGAIN-nRead=%d\n",nRead);

		epoll_ -> mod(fd,requ,(EPOLLIN | EPOLLONESHOT));
		requ -> setNoWorking();
		timerTable_ -> addTimer(requ,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,requ));
		return;
	}
	//如果解析报文出错，，就构造400报文返回
	if(!requ -> parseRequest()){
		//printf("!parseRequest\n");

		//发送400报文
		NewResponse respon(400,"",false);
		requ -> appendToOutBuffer(respon.makeResponse());
		//这里不用触发EPOLLOUT事件,直接发送400报文,然后立刻关闭连接
		int writeErrno;
		requ -> write(&writeErrno);
		requ -> setNoWorking();
		shutDownConnection(requ);
		return;
	}
	//如果解析成功,那么就发送200报文,触发EPOLLOUT事件发送;
	if(requ -> parseFinish()){
		//printf("parseFinish\n");
		NewResponse respon(200,requ -> getPath(),requ -> keepAlive());
		requ -> appendToOutBuffer(respon.makeResponse());
		epoll_->mod(fd,requ,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));	
	}
}	
//处理响应请求
void NetServer::handleResponse(NewRequest* requ){
	printf("NetServer::handleResponse\n");
	timerTable_->delTimer(requ);

	assert(requ != nullptr);
	int fd = requ -> fd();

	int toSend = requ -> writableBytes();
	if(toSend == 0){
		epoll_-> mod(fd,requ,(EPOLLIN | EPOLLONESHOT));
		requ -> setNoWorking();
		timerTable_ -> addTimer(requ,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,requ));
		return;
	}
	
	//此时outBuffer中有要发送的
	int writeErrno;
	int ret = requ -> write(&writeErrno);
	
	//非EAGAIN错误,则断开连接
	if(ret < 0 && (writeErrno != EAGAIN)){
		requ -> setNoWorking();
		shutDownConnection(requ);
		return;
	}

	if(ret < 0 && writeErrno == EAGAIN){
		epoll_-> mod(fd,requ,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));
		return;
		
	}

	if(ret == toSend){
		if(requ -> keepAlive()){
			requ -> resetParse();
			epoll_ -> mod(fd,requ,(EPOLLIN | EPOLLONESHOT));
			requ -> setNoWorking();
			timerTable_ -> addTimer(requ,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,requ));
		}else{
			requ -> setNoWorking();
			shutDownConnection(requ);
		}
		return;
	}

	epoll_ -> mod(fd,requ,(EPOLLIN | EPOLLOUT | EPOLLONESHOT));
	requ -> setNoWorking();
	timerTable_ -> addTimer(requ,CONNECT_TIMEOUT,std::bind(&NetServer::shutDownConnection,this,requ));
	return;
}



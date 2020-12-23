#include "NetServer.h"
#include "NewRequest.h"
#include "NewResponse.h"
//#include "Utils.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "SocketsHel.h"

#include <iostream>
#include <functional> // bind
#include <cassert> // assert
#include <cstring> // bzero
 
#include <unistd.h> // close, read
#include <sys/socket.h> // accept
#include <arpa/inet.h> // sockaddr_in



NetServer::NetServer(int port, int numThread) 
    : port_(port),
      listenFd_(createNonblockingHel(port_)),
      listenRequest_(new NewRequest(listenFd_)),
      epoll_(new Epoll()),
      threadPool_(new ThreadPool(numThread)),
      timerManager_(new TimerTable())
{
    assert(listenFd_ >= 0);
}

NetServer::~NetServer()
{
}

void NetServer::start()
{
    // 注册监听套接字到epoll（可读事件，ET模式）
    epoll_ -> add(listenFd_, listenRequest_.get(), (EPOLLIN | EPOLLET));
    // 注册新连接回调函数
    epoll_ -> setNewRequestCallback(std::bind(&NetServer::__acceptConnection, this));
    // 注册关闭连接回调函数
    epoll_ -> setCloseRequestCallback(std::bind(&NetServer::__closeConnection, this, std::placeholders::_1));
    // 注册请求处理回调函数
    epoll_ -> setHandleRequestCallback(std::bind(&NetServer::__doRequest, this, std::placeholders::_1));
    // 注册响应处理回调函数
    epoll_ -> setHandleResponseCallback(std::bind(&NetServer::__doResponse, this, std::placeholders::_1));

    // 事件循环
    while(1) {
        int timeMS = timerManager_ -> getNextDeadTimer();
        // 等待事件发生
        // int eventsNum = epoll_ -> wait(TIMEOUTMS);
        int eventsNum = epoll_ -> wait(timeMS);
		printf("当前活跃事件=%d\n",eventsNum);
        if(eventsNum > 0) {
            // 分发事件处理函数
            epoll_ -> handleEvent(listenFd_, threadPool_, eventsNum);
        }
        timerManager_ -> manageDeadTimers();   
    }
}

// ET
void NetServer::__acceptConnection()
{
    while(1) {
        int acceptFd = ::accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        printf("acceptFd=%d\n",acceptFd);
		if(acceptFd == -1) {
            if(errno == EAGAIN)
                break;
            printf("[NetServer::__acceptConnection] accept : %s\n", strerror(errno));
            break;
        }
        // 为新的连接套接字分配NewRequest资源
        NewRequest* request = new NewRequest(acceptFd);
        timerManager_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::__closeConnection, this, request));
        // 注册连接套接字到epoll（可读，边缘触发，保证任一时刻只被一个线程处理）
        epoll_ -> add(acceptFd, request, (EPOLLIN | EPOLLONESHOT));
    }
}

void NetServer::__closeConnection(NewRequest* request)
{
    int fd = request -> fd();
    if(request -> isWorking()) {
        return;
    }
    // printf("[NetServer::__closeConnection] connect fd = %d is closed\n", fd);
    timerManager_ -> delTimer(request);
    epoll_ -> del(fd, request, 0);
    // 释放该套接字占用的NewRequest资源，在析构函数中close(fd)
    delete request;
    request = nullptr;
}

// LT模式
void NetServer::__doRequest(NewRequest* request)
{
    timerManager_ -> delTimer(request);
    assert(request != nullptr);
    int fd = request -> fd();

    int readErrno;
    int nRead = request -> read(&readErrno);

    // read返回0表示客户端断开连接
    if(nRead == 0) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // 非EAGAIN错误，断开连接
    if(nRead < 0 && (readErrno != EAGAIN)) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    // EAGAIN错误则释放线程使用权，并监听下次可读事件epoll_ -> mod(...)
    if(nRead < 0 && readErrno == EAGAIN) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request -> setNoWorking();
        timerManager_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::__closeConnection, this, request));
        return;
    }

    // 解析报文，出错则断开连接
    if(!request -> parseRequest()) {
        // 发送400报文
        NewResponse response(400, "", false);
        request -> appendToOutBuffer(response.makeResponse());

        // XXX 立刻关闭连接了，所以就算没写完也只能写一次？
        int writeErrno;
        request -> write(&writeErrno);
        request -> setNoWorking();
        __closeConnection(request); 
        return; 
    }

    // 解析完成
    if(request -> parseFinish()) {
        NewResponse response(200, request -> getPath(), request -> keepAlive());
        request -> appendToOutBuffer(response.makeResponse());
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    }
}

// LT模式
void NetServer::__doResponse(NewRequest* request)
{
    timerManager_ -> delTimer(request);
    assert(request != nullptr);
    int fd = request -> fd();

    int toWrite = request -> writableBytes();

    if(toWrite == 0) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
        request -> setNoWorking();
        timerManager_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::__closeConnection, this, request));
        return;
    }

    int writeErrno;
    int ret = request -> write(&writeErrno);
	 // 非EAGAIN错误，断开连接
    if(ret < 0 && (writeErrno != EAGAIN)) {
        request -> setNoWorking();
        __closeConnection(request);
        return; 
    }

    if(ret < 0 && writeErrno == EAGAIN) {
        epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
        return;
    }

   
    if(ret == toWrite) {
        if(request -> keepAlive()) {
            request -> resetParse();
            epoll_ -> mod(fd, request, (EPOLLIN | EPOLLONESHOT));
            request -> setNoWorking();
            timerManager_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::__closeConnection, this, request));
        } else {
            request -> setNoWorking();
            __closeConnection(request);
        }
        return;
    }

    epoll_ -> mod(fd, request, (EPOLLIN | EPOLLOUT | EPOLLONESHOT));
    request -> setNoWorking();
    timerManager_ -> addTimer(request, CONNECT_TIMEOUT, std::bind(&NetServer::__closeConnection, this, request));
    return;
}

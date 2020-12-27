#ifndef NETSERVER__H__
#define NETSERVER__H__

#include <memory> // unique_ptr
#include <mutex>
#include <unordered_map>

#include "NewRequest.h"
#include "InitSocket.h"

#define TIMEOUTMS -1 // epoll_wait超时时间，-1表示不设超时
#define CONNECT_TIMEOUT 500 // 连接默认超时时间
#define NUM_WORKERS 4 // 线程池大小



// 前置声明，不需要包含NewRequest.h和Epoll.h

class Epoll;
class ThreadPool;
class TimerTable;
class ComAddress;

class NetServer {
public:
    NetServer(const ComAddress& serverAddr, int numThread);
    ~NetServer();
    void start(); // 启动HTTP服务器
    
private:
    void recvConnection(); // 接受新连接
    void shutDownConnection(int newFd); // 关闭连接
    void handleRequest(int newFd); // 处理HTTP请求报文，这个函数由线程池调用
    void handleResponse(int newFd);
private:
    //using ListenRequestPtr = std::unique_ptr<NewRequest>;
    using TimerTablePtr = std::unique_ptr<TimerTable>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using EpollPtr = std::unique_ptr<Epoll>;
	using NewquPtrsMap = std::unordered_map<int,NewRequestPtr>;
    int port_; // 监听端口
    int listenFd_; // 监听套接字

    NewRequestPtr serverRequest_; // 监听套接字的NewRequest实例
    InitSocket serverSocket_;

	EpollPtr epoll_; // epoll实例
    ThreadPoolPtr threadPool_; // 线程池
    TimerTablePtr timerTable_; // 定时器管理器
	NewquPtrsMap ptrsMap_;
	
}; // class NetServer



#endif

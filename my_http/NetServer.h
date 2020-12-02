#ifndef NETSERVER__H__
#define NETSERVER__H__

#include <memory> // unique_ptr
#include <mutex>

#define TIMEOUTMS -1 // epoll_wait超时时间，-1表示不设超时
#define CONNECT_TIMEOUT 500 // 连接默认超时时间
#define NUM_WORKERS 4 // 线程池大小



// 前置声明，不需要包含NewRequest.h和Epoll.h
class NewRequest;
class Epoll;
class ThreadPool;
class TimerTable;


class NetServer {
public:
    NetServer(int port, int numThread);
    ~NetServer();
    void start(); // 启动HTTP服务器
    
private:
    void recvConnection(); // 接受新连接
    void shutDownConnection(NewRequest* request); // 关闭连接
    void handleRequest(NewRequest* request); // 处理HTTP请求报文，这个函数由线程池调用
    void handleResponse(NewRequest* request);
private:
    using ListenRequestPtr = std::unique_ptr<NewRequest>;
    using TimerTablePtr = std::unique_ptr<TimerTable>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using EpollPtr = std::unique_ptr<Epoll>;

    int port_; // 监听端口
    int listenFd_; // 监听套接字
    ListenRequestPtr serverRequest_; // 监听套接字的NewRequest实例
    EpollPtr epoll_; // epoll实例
    ThreadPoolPtr threadPool_; // 线程池
    TimerTablePtr timerTable_; // 定时器管理器
}; // class NetServer



#endif

#ifndef NETSERVER__H__
#define NETSERVER__H__

#include <memory> // shared_ptr unique_ptr
#include <mutex>

#define TIMEOUTMS -1
#define CONNECT_TIMEOUT 500


class NewRequest;
class Epoll;
class ThreadPool;
class TimerTable;

class NetServer{
public:
	NetServer(int port,int numThread);
	~NetServer();

	void start();    //开启服务器

private:
	void __acceptConnection();   //接受新连接
	void __closeConnection(NewRequest* requ);  //关闭连接
	void __doRequest(NewRequest* requ);    //处理HTTP请求报文
	void __doResponse(NewRequest* requ);   //处理回复内容
private:
	using ListenRequestPtr = std::unique_ptr<NewRequest>;
	using TimerTablePtr = std::unique_ptr<TimerTable>;
	using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
	using EpollPtr = std::unique_ptr<Epoll>;
	
	int port_; //该服务器监听的端口	
	int listenFd_; //该服务器监听的套接字
	ListenRequestPtr listenRequest_;
	EpollPtr epoll_;
	ThreadPoolPtr threadPool_;
	TimerTablePtr timerManager_;

	//static const int epollTime_;
	//static const int defaultTimeouts_;
	//static const int poolSize_;
};

class NetConn{
public:
	NetConn(){}
	~NetConn(){}	

};
using NetConnPtr = std::shared_ptr<NetConn>;
using TimeoutsCallback = std::function<void()>;



#endif

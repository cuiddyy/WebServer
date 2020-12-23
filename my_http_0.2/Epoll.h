#ifndef EPOLL__H__
#define EPOLL__H__

#include <functional> //function
#include <memory> //shared_ptr
#include <vector>

#include <sys/epoll.h> //epoll

#include "NewRequest.h"

class ThreadPool;

class Epoll{
public:
	using NewRequestCallback = std::function<void()>;
	using CloseRequestCallback = std::function<void(int)>;
	using HandleRequestCallback = std::function<void(int)>;
	using HandleResponseCallback = std::function<void(int)>;
	
	Epoll();
	~Epoll();
	int add(int fd,NewRequestPtr requPtr,int events); //注册新的fd
	int mod(int fd,NewRequestPtr requPtr,int events); //修改fd的监测事件
	int del(int fd,NewRequestPtr requPtr,int events); //移除fd
	int wait(int timeOuts);//等待事件的发生,返回活跃fd的数量.
	void setNewRequestCallback(const NewRequestCallback& cb){onNewReuquest_ = cb;}//绑定新连接处理回调函数
	void setCloseRequestCallback(const CloseRequestCallback& cb){onCloseRequest_ = cb;}//绑定关闭连接回调函数
	void setHandleRequestCallback(const HandleRequestCallback& cb){onHandleRequest_ = cb;}//绑定请求回调函数
	void setHandleResponseCallback(const HandleResponseCallback& cb){onHandleResponse_ = cb;}//绑定相应回调函数
	void handleEvent(int listenFd,std::shared_ptr<ThreadPool>& threadPool,int eventNum);//处理事件函数

private:
	const int maxEvents_;
	using EventList = std::vector<struct epoll_event>;
	int epollFd_;
	EventList events_;
	NewRequestCallback onNewReuquest_;
	CloseRequestCallback onCloseRequest_;
	HandleRequestCallback onHandleRequest_;
	HandleResponseCallback onHandleResponse_;

};





#endif

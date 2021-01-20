#include "Epoll.h"
#include "ThreadPool.h"


#include <iostream>
#include <cassert>
#include <cstring> //perror

#include <unistd.h> //close(fd)



Epoll::Epoll(): maxEvents_(1024),epollFd_(::epoll_create1(EPOLL_CLOEXEC)),events_(maxEvents_) {
	assert(epollFd_ >=0 );
}

Epoll::~Epoll(){
	::close(epollFd_);
}

int Epoll::add(int fd,NewRequestPtr requPtr,int events){
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(requPtr.get());
	event.events = events;
	int ret = ::epoll_ctl(epollFd_,EPOLL_CTL_ADD,fd,&event);
	return ret;
}

int Epoll::mod(int fd,NewRequestPtr request,int events){
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(request.get());
	event.events = events;
	int ret = ::epoll_ctl(epollFd_,EPOLL_CTL_MOD,fd,&event);

	return ret;
}

int Epoll::del(int fd,NewRequestPtr request,int events){
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(request.get());
	event.events = events;
	int ret = ::epoll_ctl(epollFd_,EPOLL_CTL_DEL,fd,&event);
	return ret;
}

int Epoll::wait(int timeoutMs){
	int eventNum = ::epoll_wait(epollFd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
	printf("Epoll:wait__eventNum=%d\n",eventNum);
	if(eventNum < 0){
		printf("Epoll::wait() : %s\n",strerror(errno));
	}

	return eventNum;
}

void Epoll::handleEvent(int listenFd,std::shared_ptr<ThreadPool>& threadPool,int eventNum){
	printf("Epoll::handleEvent : eventNum=%d\n",eventNum);
	assert(eventNum > 0);
	for(int i=0;i<eventNum;i++){
		NewRequest* request = static_cast<NewRequest*>(events_[i].data.ptr);
		int fd = request -> fd();
		if(fd == listenFd){
			printf("新连接事件\n");
			onNewReuquest_();
		}else{
			if(events_[i].events & EPOLLERR || events_[i].events & EPOLLHUP||
			   (!events_[i].events & EPOLLIN)){
				printf("Errno事件\n");
				request->setNoWorking();
				onCloseRequest_(fd);
			}else if(events_[i].events & EPOLLIN){
				request->setWorking();
				printf("EPOLLIN事件__fd=%d\n",fd);	
				threadPool -> addJob(std::bind(onHandleRequest_,fd));
			}else if(events_[i].events & EPOLLOUT) {
				printf("EPOLLOUT事件__fd=%d\n",fd);
				request -> setWorking();
				threadPool -> addJob(std::bind(onHandleResponse_,fd));
			}else{
				printf("[Epoll::handleEvent] unexpected event\n");
			}
		}
	}
	return;
}

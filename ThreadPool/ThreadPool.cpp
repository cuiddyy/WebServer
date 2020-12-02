#include "ThreadPool.h"
#include <iostream>

using JobFunc = std::function<void()>;


int ThreadPool::addTask(JobFunc&& func){
	std::unique_lock<std::mutex>lock(mx_);
	int ret = tasks_.push(func);
	if(ret > 0){
		std::cout << "添加任务成功,当前任务数为: "<< getTaskNum() << std::endl;
		cond_.notify_one();
	}
	return ret;
}

void ThreadPool::start(){
	if(!processing_){
		std::unique_lock<std::mutex>lock(mx_);
		processing_ = true;

		threads_.reserve(numWorkers_);
		for(int i=0;i<numWorkers_;i++){
			threads_.emplace_back(new std::thread(std::bind(&ThreadPool::run,this)));
		}
	}

}

void ThreadPool::run(){
	while(processing_){
		//std::cout << "run" << std::endl;
		JobFunc jf;
		std::unique_lock<std::mutex>lock(mx_);
		{
			while(processing_ && tasks_.getTaskNum() == 0){
				cond_.wait(lock);
			}
			if(!processing_ && tasks_.getTaskNum() == 0){
				return;
			}
			jf = tasks_.pop();
		}
		if(jf)
			jf();
	}
}

void ThreadPool::stop(){
	{
		std::unique_lock<std::mutex>lock(mx_);
		processing_ = false;
	}

	cond_.notify_all();//通知所有线程当前线程池要停止了,不要再接收新的任务.

	for(auto & th:threads_){
		std::cout << "线程 " << th->get_id() << "销毁" << std::endl;
		th->join();
		if(th != nullptr){
			delete th;
			th = nullptr;
		}
	}

	std::cout << "当前线程已经全部销毁" << std::endl;

}

int ThreadPool::getTaskNum(){
	return tasks_.getTaskNum();
}

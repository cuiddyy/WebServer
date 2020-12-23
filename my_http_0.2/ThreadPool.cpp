#include "ThreadPool.h"

#include <iostream>
#include <cassert>


ThreadPool::ThreadPool(int handleNums):handling_(true){
	handleNums_ = handleNums <= 0? 1:handleNums;
	for(int i=0;i<handleNums_;i++){
		thread_.emplace_back([this](){
			while(1){
				HandleFunc oneJob;
				{
					std::unique_lock<std::mutex>lock(mutex_);
					while(handling_ && handleJobs_.empty())
						cond_.wait(lock);
					if(!handling_ && handleJobs_.empty()){
						return;
					}
					oneJob = handleJobs_.front();
					handleJobs_.pop();
				}
				if(oneJob){
					oneJob();
				}
			}
		});
	}
}

ThreadPool::~ThreadPool(){
	{
		std::unique_lock<std::mutex>lock(mutex_);
		handling_ = false;
	}
	
}

void ThreadPool::addJob(const HandleFunc& func){
	{
		std::unique_lock<std::mutex>lock(mutex_);
		handleJobs_.push(func);

	}

	cond_.notify_one();
}

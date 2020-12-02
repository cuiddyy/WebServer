#ifndef THREADPOOL__H__
#define THREADPOOL__H__

#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include "Task.h"

using JobFunc = std::function<void()>;

class ThreadPool{

public:
	ThreadPool(int numWorkers):numWorkers_(numWorkers),processing_(false){}
	~ThreadPool(){
		if(processing_) stop();
	}
	int addTask(JobFunc&& jf);//在任务队列中添加任务
	void start();//开启线程池
	void stop(); //关闭线程池
	int getTaskNum();//获得当前任务队列中的任务数
	
private:
	void run(); //线程工作函数

	int numWorkers_; //线程池的线程数量
	std::mutex mx_; //锁
	std::condition_variable cond_;//条件变量
	std::vector<std::thread*> threads_;
	bool processing_;
	Task<JobFunc>tasks_;

};


#endif

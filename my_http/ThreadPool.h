#ifndef THREADPOOL__H__
#define THREADPOOL__H__

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

using  HandleFunc = std::function<void()>;

class ThreadPool{

public:
	ThreadPool(int handleNums_);
	~ThreadPool();
	void addJob(const HandleFunc& job);

private:
	std::vector<std::thread>thread_;
	std::queue<HandleFunc> handleJobs_;
	std::mutex mutex_;
	std::condition_variable cond_;
	bool handling_;
	int handleNums_;

};





#endif

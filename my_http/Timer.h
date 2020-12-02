#ifndef TIMER__H__
#define TIMER__H__

#include <functional>
#include <chrono>
#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <mutex>

using TimeoutCallback = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using Timestamp = Clock::time_point;

class NewRequest;

class Timer{
public:
	Timer(const Timestamp& deadTime,const TimeoutCallback& timerCallback)
	:deadTime_(deadTime),
	del_(false),
	timerCallback_(timerCallback){}
	~Timer(){}
	void del() {del_ = true;}
	bool isDeleted() {return del_;}
	Timestamp getDeadTime() const {return deadTime_;}
	void runCalllback() {timerCallback_();}

private:
	Timestamp deadTime_;
	bool del_;
	TimeoutCallback timerCallback_;
};

class timerCmp{
public:
	bool operator()(Timer* t1,Timer* t2){
		assert(t1 != nullptr && t2 != nullptr);
		return (t1->getDeadTime()) > (t2->getDeadTime());
	}
};

class TimerTable{

public:
	TimerTable():now_(Clock::now()){}
	~TimerTable(){}
	void updateTimer(){now_ = Clock::now();}
	void addTimer(NewRequest* requ,const int& deadTime,const TimeoutCallback& cb);//deadTime单位为ms
	void delTimer(NewRequest* requ);
	void manageDeadTimers();
	int getNextDeadTimer();


private:
	using TimerQueue = std::priority_queue<Timer*,std::vector<Timer*>,timerCmp>;
	TimerQueue timersQ_;
	Timestamp now_;
	std::mutex lock_;
};




#endif

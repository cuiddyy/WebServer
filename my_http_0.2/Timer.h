#ifndef TIMER__H__
#define TIMER__H__

#include "NewRequest.h"


#include <functional>
#include <chrono>
#include <queue>
#include <vector>
#include <iostream>
#include <cassert>
#include <mutex>
#include <memory>




using TimeoutCallback = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using Timestamp = Clock::time_point;


class Timer{
public:
	Timer(const Timestamp& deadTime,const TimeoutCallback& timerCallback)
	:deadTime_(deadTime),
	del_(false),
	timerCallback_(timerCallback){}
	~Timer(){}
	Timestamp getDeadTime() const {return deadTime_;}
	void runCalllback() {timerCallback_();}
	bool isDel() const {return del_;}
	void Del(){del_ = true;}
private:
	Timestamp deadTime_;
	TimeoutCallback timerCallback_;
	bool del_;
};


using TimerPtr = std::shared_ptr<Timer>;

class timerCmp{
public:
	bool operator()(const TimerPtr &t1,const TimerPtr &t2){
		assert(t1 != nullptr && t2 != nullptr);
		return (t1->getDeadTime()) > (t2->getDeadTime());
	}
};


class TimerTable{

public:
	TimerTable():now_(Clock::now()){}
	~TimerTable(){}
	void updateTimer(){now_ = Clock::now();}
	void addTimer(NewRequestPtr requPtr,const int& deadTime,const TimeoutCallback& cb);//deadTime单位为ms
	void delTimer(NewRequestPtr requPtr);
	void manageDeadTimers();
	int getNextDeadTimer();


private:
	using TimerQueue = std::priority_queue<TimerPtr,std::vector<TimerPtr>,timerCmp>;
	TimerQueue timersQ_;
	Timestamp now_;
	std::mutex lock_;
};




#endif

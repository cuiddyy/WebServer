#include "Timer.h"
#include "NewRequest.h"

#include <cassert>

void TimerTable::addTimer(NewRequest* requ,const int& deadTime,const TimeoutCallback& cb){
	std::unique_lock<std::mutex>lock(lock_);
	assert(requ != nullptr);
	updateTimer();
	Timer* timer = new Timer(now_+(MS)deadTime,cb);
	timersQ_.push(timer);
	if(requ->getTimer() != nullptr){
		delTimer(requ);
	}
	requ->setTimer(timer);
	
}

void TimerTable::delTimer(NewRequest* requ){
	assert(requ!=nullptr);
	Timer* timer = requ->getTimer();
	if(timer == nullptr){return;}
	timer->del();
	requ->setTimer(nullptr);
}	

int TimerTable::getNextDeadTimer(){
	std::unique_lock<std::mutex>lock(lock_);
	updateTimer();
	int res = -1;
	while(!timersQ_.empty()){
		Timer* timer = timersQ_.top();
		assert(timer!=nullptr);
		if(timer->isDeleted()){
			timersQ_.pop();
			delete timer;
			continue;
		}
		res = std::chrono::duration_cast<MS>(timer -> getDeadTime() -now_).count();
		res = res < 0 ? 0:res;
		break;
	}
	return res;
}
void TimerTable::manageDeadTimers(){
	printf("整理前TimerTable中timers的大小为=%d\n",timersQ_.size());

	std::unique_lock<std::mutex>lock(lock_);
	updateTimer();
	while(!timersQ_.empty()){
		Timer* timer = timersQ_.top();
		assert(timer!=nullptr);
		if(timer->isDeleted()){
			timersQ_.pop();
			delete timer;
			continue;
		}
		if(std::chrono::duration_cast<MS>(timer -> getDeadTime() - now_).count() > 0){
			return;
		}
		timer->runCalllback();
		timersQ_.pop();
		delete timer;

	}

	printf("整理后TimerTable中timers的大小为=%d\n",timersQ_.size());
}



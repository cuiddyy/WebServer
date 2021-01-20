#include "Timer.h"
#include "NewRequest.h"

#include <cassert>

void TimerTable::addTimer(NewRequestPtr requPtr,const int& deadTime,const TimeoutCallback& cb){
	std::unique_lock<std::mutex>lock(lock_);
	assert(requPtr != nullptr);
	updateTimer();
	TimerPtr tp(new Timer(now_+(MS)deadTime,cb));

	timersQ_.push(tp);
	requPtr->setTimer(tp);
	
}

void TimerTable::delTimer(NewRequestPtr requPtr){
	//printf("TimerTable::delTimer__fd=%d\n",requPtr->fd());
	assert(requPtr.use_count() > 0);
	std::weak_ptr<Timer> wtptr = requPtr->getTimer();
	//这里之所以会有引用数>0的情况,是因为可能存在因为异常导致删除连接的情况,需要利用Del()告诉TimerTable删除作废的Timer.
	if(wtptr.use_count() > 0)
		wtptr.lock()->Del();
}	

int TimerTable::getNextDeadTimer(){
	std::unique_lock<std::mutex>lock(lock_);
	updateTimer();
	int res = -1;
	while(!timersQ_.empty()){
		TimerPtr timerPtr = timersQ_.top();
		assert(timerPtr.use_count() > 0);
		if(timerPtr->isDel()){
			timersQ_.pop();
			continue;
		}
		res = std::chrono::duration_cast<MS>(timerPtr -> getDeadTime() -now_).count();
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
		TimerPtr timerPtr = timersQ_.top();
		assert(timerPtr.use_count() > 0);
		if(timerPtr->isDel()){
			timersQ_.pop();
			continue;
		}
		if(std::chrono::duration_cast<MS>(timerPtr -> getDeadTime() - now_).count() > 0){
			return;
		}
		timerPtr->runCalllback();//将这个Timer对应的NewRequestPtr erase掉,删除对应fd的监控.
		timersQ_.pop();
		
	}

	printf("整理后TimerTable中timers的大小为=%d\n",timersQ_.size());
}



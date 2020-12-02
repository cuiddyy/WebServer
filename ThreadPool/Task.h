#ifndef  TASK__H__
#define	 TASK__H__

#include <queue>


template <typename T>
class Task{
public:
	Task(){}
	~Task(){}
	int push(T t);
	int getTaskNum();
	T pop();
private:
	std::queue<T>tasks_;
};


template<typename T>
int Task<T>::push(T t){
	try{
		tasks_.emplace(t);
	}catch(std::exception e){
		throw e;
		return -1;
	}
	return 1;
}

template <typename T>
int Task<T>::getTaskNum(){
	return tasks_.size();
}

template<typename T>
T Task<T>::pop(){
	T tmp;
	if(!tasks_.empty()){
		tmp = tasks_.front();
		tasks_.pop();
	}
	return tmp;
}


#endif

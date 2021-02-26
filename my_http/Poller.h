#ifndef POLLER__H__
#define POLLER__H__

#include "BasePoll.h"

class Poller:public BasePoll{
public:
	Poller();
	~Poller();
	int pollRevents(int timeOuts);
	int addEvents(int fd,int events);
	int updateEvents(int fd,int events);
	int removeEvents(int fd,int events);
	
private:
	using PollFds = std::vector<struct pollfd>;
	PollFds pollfds_;
};







#endif

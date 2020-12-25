#ifndef POLLER__H__
#define POLLER__H__

using 


class Poller{
public:
	Poller();
	~Poller();
	int pollRevents(int timeOuts);
	int addEvents(int fd,int events);
	int updateEvents(int fd,int events);
	int removeEvents(int fd,int events);

};







#endif

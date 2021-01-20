#ifndef INITSOCKET__H__
#define INITSOCKET__H__

//#include "muduo/base/noncopyable.h"

class ComAddress;
struct tcp_info;

class InitSocket{

public:
	explicit InitSocket(int socketfd):socketfd_(socketfd){}

	~InitSocket();

	int fd() const {return socketfd_;}

	bool getTcpInfo(struct tcp_info*)const;
	bool getTcpInfoString(char* buf,int len)const;

	void bindAddr(const ComAddress& locaddr);
	void startListen();
	
	int acceptConn(ComAddress* peeraddr);

	void closeWrite();

	void setTcpConNoDelay(bool tar);

	void setReuseAddr(bool tar);

	void setReusePort(bool tar);
	
	void setKeepAlive(bool tar);

private:
	const int socketfd_;

};



#endif

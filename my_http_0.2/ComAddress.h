#ifndef COMADDRESS__H__
#define COMADDRESS__H__

#include <netinet/in.h>


#include <string>

#include "SocketsHel.h"
#include "Types.h"
//const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
//int bbbb = aaaa;


class ComAddress{

public:
	explicit ComAddress(uint16_t port = 0);
	ComAddress(const std::string ip,uint16_t port);
	explicit ComAddress(const struct sockaddr_in& addr):addr_(addr){}
	sa_family_t family() const {return addr_.sin_family;}

	std::string dip_stctos() const;
	std::string dipport_stctos() const;
    uint16_t dport_stctoin() const;
	uint16_t addr_port() const;
	const struct sockaddr* getSockAddr() const {
		return sockaddr_cast(&addr_);
	}
	void setSockAddr(const struct sockaddr_in& addr){addr_ = addr;}	
private:
	struct sockaddr_in addr_;	
};



#endif

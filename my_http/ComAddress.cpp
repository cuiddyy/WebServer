#include "ComAddress.h"
//#include "SocketsHel.h"


#include <endian.h>
//#include "Types.h"

//#include "muduo/base/Logging.h"
//#include "muduo/net/Endian.h"


//#include "SocketsHel.cc"

#include <netdb.h>
#include <netinet/in.h>




ComAddress::ComAddress(uint16_t port){
	//addr_.sin_port = port;
}

ComAddress::ComAddress(const std::string ip,uint16_t port){
	
	//memZero(&addr_,sizeof addr_);
	//::bzero((char*)&addr_,sizeof(addr_));
	ipport_stostc(ip.c_str(),port,&addr_);
	/*char buf[64];
	uint16_t port1 = ntohs(addr_.sin_port);
	inet_ntop(AF_INET,&addr_.sin_addr.s_addr,buf,64);
	*/
	//printf("ip=%s_____port=%d\n",buf,port1);
}

std::string ComAddress::dipport_stctos() const {
	char buf[64] = "";
	ipport_stctos(buf,sizeof buf,getSockAddr());
	return buf;
}

std::string ComAddress::dip_stctos() const {
	char buf[64] = "";
	ip_stctos(buf,sizeof buf,getSockAddr());
	return buf;
}

uint16_t ComAddress::dport_stctoin() const {
	return be16toh(addr_port()); 
}

uint16_t ComAddress::addr_port() const {
	return addr_.sin_port;
}

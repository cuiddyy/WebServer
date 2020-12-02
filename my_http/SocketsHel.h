#ifndef SOCKETSHEL__H__
#define SOCKETSHEL__H__

#include <arpa/inet.h>
#include <cstring> //memset

inline void memZero(void* p,size_t n){
	memset(p,0,n);
}

template<typename Des,typename Sou>
inline Des implicit_cast(Sou const &f){
	return f;
}

void setnbAndcoeHel(int socketfd);
int createNonblockingHel(int port);

//void setReuseAddr(bool tar);
//void setReusePort(bool tar);
//void setKeepAlive(bool tar);

int connectHel(int socketfd,const struct sockaddr* addr);
void bindHel(int socketfd,const struct sockaddr* addr);
void listenHel(int socketfd);
int acceptHel(int socketfd,struct sockaddr_in* addr);
ssize_t readHel(int socketfd,void *buf,size_t count);
ssize_t readvHel(int socketfd,const struct iovec *iov,int iovcnt);
ssize_t writeHel(int socketfd,const void *buf,size_t count);
void closeHel(int socketfd);
void closeWriteHel(int socketfd);
void setSockAddrHel(struct sockaddr_in* addr);
void ipport_stctos(char *ip,size_t size,const struct sockaddr* addr);
void ip_stctos(char *ip,size_t size,const struct sockaddr* addr);

void ipport_stostc(const char *ip,uint16_t port,struct sockaddr_in* addr);

int getSocketError(int socketfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr);
bool isSelfConnect(int socketfd);
struct sockaddr_in getLocalAddr(int socketfd);
struct sockaddr_in getPeerAddr(int socketfd);








#endif

#include "SocketsHel.h"



#include <cstring> // strlen,memset

#include <cassert> //assert
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>
#include <endian.h>



const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr){
	return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr){
	return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr){
	return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}
struct sockaddr_in* sockaddr_in_cast(struct sockaddr* addr){
	return static_cast<struct sockaddr_in*>(implicit_cast<void*>(addr));
}







void setnbAndcoeHel(int socketfd){
	int flags = ::fcntl(socketfd,F_GETFL,0);
	flags |= O_NONBLOCK;
	int res = ::fcntl(socketfd,F_SETFL,flags);
	flags = ::fcntl(socketfd,F_GETFL,0);
	flags |= FD_CLOEXEC;
	res = ::fcntl(socketfd,F_SETFL,flags);

	(void)res;
}

int createNonblockingHel(int port){
	port = ((port <= 1024) || (port >= 65535)) ? 6666:port;
	
	//创建套接字
	int socketfd = ::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK ,0);
	if(socketfd<0){
		printf("createNonblockingHel\n");
	}
	//避免addr已经被use
	int optval = 1;
	if(::setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof(optval))) == -1){
		printf("SocketsHel::createNonblockingHel fd=%d setsockopt : %s\n",socketfd,strerror(errno));
		return -1;
	}
    //bind套接字和ip,端口	
	struct sockaddr_in addr;
	::bzero((char*)&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	addr.sin_port = ::htons((unsigned short)port);
	if(::bind(socketfd,sockaddr_cast(&addr),static_cast<socklen_t>(sizeof(addr)))==-1){
		printf("SocketsHel::createNonblockingHel fd=%d bind:%s\n",socketfd,strerror(errno));
	}

	//开始监听
	listenHel(socketfd);
	
	if(socketfd == -1){
		::close(socketfd);
		return -1;
	}

	return socketfd;
}


void listenHel(int socketfd){
	int res = ::listen(socketfd,1024);
	if(res < 0){
		printf("listenHel\n");
	}
}

int acceptHel(int socketfd,struct sockaddr_in* addr){
	socklen_t addlen = static_cast<socklen_t>(sizeof *addr);
	int accfd = ::accept(socketfd,sockaddr_cast(addr),&addlen);
	setnbAndcoeHel(accfd);
	if(accfd < 0){
		//int savedErrno = errno;
		printf("acceptHel\n");
	}
	return accfd;
}
int connectHel(int socketfd,const struct sockaddr* addr){
	return ::connect(socketfd,addr,static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t readHel(int socketfd,void *buf,int count){
	return ::read(socketfd,buf,count);
}

ssize_t readvHel(int socketfd,const struct iovec *iov,int iovcnt){
		return ::readv(socketfd,iov,iovcnt);
}

ssize_t writeHel(int socketfd,const void *buf,size_t count){
	return ::write(socketfd,buf,count);
}

void closeHel(int socketfd){
	int res = ::close(socketfd);
	if(res < 0){
		printf("closeHel\n");
	}
}

void closeWriteHel(int socketfd){
	int res = ::shutdown(socketfd,SHUT_WR);
	if(res < 0){
		printf("closeWrite\n");
	}
}

void ipport_stctos(char *buf,size_t size,const struct sockaddr* addr){
	ip_stctos(buf,size,addr);
	size_t cur = ::strlen(buf);
	const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
	uint16_t port = be16toh(addr4->sin_port);
	assert(size > cur);
	snprintf(buf+cur,size-cur,":%u",port);
}

void ip_stctos(char *buf,size_t size,const struct sockaddr* addr){
	if(addr->sa_family == AF_INET){
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET,&addr4->sin_addr,buf,static_cast<socklen_t>(size));
	}else if(addr->sa_family == AF_INET6){
		printf("ip_stctos receives ip6\n");
	}
}

void ipport_stostc(const char* ip,uint16_t port,struct sockaddr_in* addr){
	addr->sin_family = AF_INET;
	addr->sin_port = htobe16(port);
	if(::inet_pton(AF_INET,ip,&addr->sin_addr) <= 0){
		printf("ipport_stostc\n");
	}
}

int getSocketError(int socketfd){
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if(::getsockopt(socketfd,SOL_SOCKET,SO_ERROR,&optval,&optlen) < 0){
		return errno;
	}else{
		return optval;
	}
}

struct sockaddr_in getLocalAddr(int socketfd){
	struct sockaddr_in localaddr;
	memZero(&localaddr,sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if(::getsockname(socketfd,sockaddr_cast(&localaddr),&addrlen) < 0){
		printf("getLocalAddr\n");
	}

	return localaddr;

}

struct sockaddr_in getPeerAddr(int socketfd){
	struct sockaddr_in peeraddr;
	memZero(&peeraddr,sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if(::getpeername(socketfd,sockaddr_cast(&peeraddr),&addrlen) < 0){
		printf("getPeerAddr\n");
	}
	return peeraddr;
}

bool isSelfConnect(int socketfd){
	struct sockaddr_in localaddr = getLocalAddr(socketfd);
	struct sockaddr_in peeraddr = getPeerAddr(socketfd);
	if(localaddr.sin_family == AF_INET){
		const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
		const struct sockaddr_in* paddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
		return laddr4->sin_port == paddr4->sin_port && laddr4->sin_addr.s_addr == paddr4->sin_addr.s_addr;
	}
	return false;
}


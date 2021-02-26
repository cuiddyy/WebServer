#include "InitSocket.h"


#include "ComAddress.h"
#include "SocketsHel.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>    //snprintf




InitSocket::~InitSocket(){
	closeHel(socketfd_);
}
void InitSocket::bindAddr(const ComAddress& addr){
	bindHel(socketfd_,addr.getSockAddr());	
}

bool InitSocket::getTcpInfo(struct tcp_info* tcp_data) const {
	socklen_t len=sizeof(*tcp_data);
	memZero(tcp_data,len);
	return ::getsockopt(socketfd_,SOL_TCP,TCP_INFO,tcp_data,&len) == 0;
}

bool InitSocket::getTcpInfoString(char* buf,int len) const {
	struct tcp_info tcpi_data;
	bool res = getTcpInfo(&tcpi_data);

	if(res){
		snprintf(buf, len, "unrecovered=%u "
				"rto=%u ato=%u snd_mss=%u rcv_mss=%u "
				"lost=%u retrans=%u rtt=%u rttvar=%u "
				"sshthresh=%u cwnd=%u total_retrans=%u",
				 tcpi_data.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
				 tcpi_data.tcpi_rto,          // Retransmit timeout in usec
				 tcpi_data.tcpi_ato,          // Predicted tick of soft clock in usec
				 tcpi_data.tcpi_snd_mss,
			     tcpi_data.tcpi_rcv_mss,
			     tcpi_data.tcpi_lost,         // Lost packets
                 tcpi_data.tcpi_retrans,      // Retransmitted packets out
                 tcpi_data.tcpi_rtt,          // Smoothed round trip time in usec
                 tcpi_data.tcpi_rttvar,       // Medium deviation
                 tcpi_data.tcpi_snd_ssthresh,
	             tcpi_data.tcpi_snd_cwnd,
	             tcpi_data.tcpi_total_retrans);  // Total retransmits for entire connection
	}
	return res;
}
void InitSocket::startListen(){
	listenHel(socketfd_);
}

int InitSocket::acceptConn(ComAddress* peeraddr){
	struct sockaddr_in addr;
	memZero(&addr,sizeof addr);
	int recfd = acceptHel(socketfd_,&addr);	
	if(recfd >= 0){
		peeraddr->setSockAddr(addr); 
	}

	return recfd;
}

void InitSocket::closeWrite(){
	closeWriteHel(socketfd_);
}

void InitSocket::setTcpConNoDelay(bool tar){
	int optval = tar?1:0;
	::setsockopt(socketfd_,IPPROTO_TCP,TCP_NODELAY,&optval,static_cast<socklen_t>(sizeof optval));
}

void InitSocket::setReuseAddr(bool tar){
	int optval = tar?1:0;
	int ans = ::setsockopt(socketfd_,SOL_SOCKET,SO_REUSEADDR,&optval,static_cast<socklen_t>(sizeof(optval)));
	if(ans<0){
		printf("InitSocket::setReuseAddr %s\n",strerror(errno));
	}

}

void InitSocket::setReusePort(bool tar){
	int optval=tar?1:0;
	int res = ::setsockopt(socketfd_,SOL_SOCKET,SO_REUSEPORT,&optval,static_cast<socklen_t>(sizeof(optval)));
	if(res < 0 && tar){
		printf("InitSocket::setReusePort failed.\n");

	}
}

void InitSocket::setKeepAlive(bool tar){
	int optval = tar?1:0;
	::setsockopt(socketfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,static_cast<socklen_t>(sizeof(optval)));

}

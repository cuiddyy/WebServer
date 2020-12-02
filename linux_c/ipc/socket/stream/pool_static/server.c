#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "proto.h"

#define IPSTRSIZE 40
#define PROCNUM 4
#define BUFFSIZE 1024
static void server_loop(int sd);
static void server_job(int sd){
	char buf[BUFFSIZE];
	int len;
	len = sprintf(buf,FMT_STAMP,(long long)time(NULL));
	if(send(sd,buf,len,0)<0){
		perror("send()");
		exit(1);
	}

}

int main(){
	int sd,i;	
	struct sockaddr_in laddr;
	pid_t pid;
	sd = socket(AF_INET,SOCK_STREAM,0/*0等价于IPPROTO_TCP,IPPROTO_SCTP*/);
	if(sd<0){
		perror("socket()");
		exit(1);
	}
	//防止端口没有及时释放
	int val=1;
	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0){
		perror("setsocketopt()");
		exit(1);		
	}

	laddr.sin_family=AF_INET;
	laddr.sin_port=htons(atoi(SERVERPORT));
	inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);

	if(bind(sd,(void *)&laddr,sizeof(laddr))<0){
		perror("bind()");
		exit(1);
	
	}

	if(listen(sd,200)<0){
		perror("listen()");
		exit(1);
	}
	
	for(i=0;i<PROCNUM;i++){
		pid = fork();
		if(pid<0){
			perror("fork()");
			exit(1);
		}
		if(pid==0){
			server_loop(sd);
			exit(0);
		}
	}
	for(i=0;i<PROCNUM;i++){
		wait(NULL);
	}
	close(sd);
	exit(0);


	close(sd);


	exit(0);
}

static void server_loop(int sd){
	struct sockaddr_in raddr;
	socklen_t raddr_len;	
	int newsd;
	char ipstr[IPSTRSIZE];
	raddr_len=sizeof(raddr);
	while(1){
		
		newsd = accept(sd,(void *)&raddr,&raddr_len);
		if(newsd<0){
			perror("accept()");
			exit(1);
		}
		
		inet_ntop(AF_INET,&raddr.sin_addr,ipstr,IPSTRSIZE);
		printf("[%d]Client:%s:%d\n",getpid(),ipstr,ntohs(raddr.sin_port));
		
		server_job(newsd);
		
		close(newsd);
		
	}

}

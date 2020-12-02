#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>



#define BUFSIZE 1024




int main(int argc,char *argv[]){
	
	int sd,len;
	long long stamp;
	FILE *fp;
	struct sockaddr_in raddr;
	char rbuf[BUFSIZE];
	if(argc<2){
		fprintf(stderr,"Usage...\n");
		exit(1);
	}
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd<0){
	
		perror("socket()");
		exit(1);
	}

	//bind();
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons(80);//hton:主机序到网络序的转换
	inet_pton(AF_INET,argv[1],&raddr.sin_addr);
	if(connect(sd,(void *)&raddr,sizeof(raddr))<0){
		perror("connect()");
		exit(1);
	}
	//这时候sd已经被复写
	fp = fdopen(sd,"r+");
	if(fp == NULL){
		perror("fopen()");
		exit(1);
	}
	
	fprintf(fp,"GET /test.jpg\r\n\r\n");
	fflush(fp);
	while(1){
		len = fread(rbuf,1,BUFSIZE,fp);
		if(len<=0)
			break;
		fwrite(rbuf,1,len,stdout);
	}
	
	fclose(fp);


	exit(0);
}

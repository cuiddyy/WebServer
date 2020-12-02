#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>



#include "proto.h"





int main(int argc,char *argv[]){
	
	int sd;
	long long stamp;
	FILE *fp;
	struct sockaddr_in raddr;
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
	raddr.sin_port = htons(atoi(SERVERPORT));//hton:主机序到网络序的转换
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

	if(fscanf(fp,FMT_STAMP,&stamp)<1)
		fprintf(stderr,"Bad format!\n");
	else
		fprintf(stdout,"stamp = %lld\n",stamp);
	fclose(fp);


	exit(0);
}

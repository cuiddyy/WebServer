#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include "mytbf.h"
#define CPS 10
#define BUFFSIZE 1024
#define BURST 100

int main(int argc,char **argv){
	int sfd,dfd=1,size;
	ssize_t len,ret;
	char buf[BUFFSIZE];
	struct mytbf_t *tbf;
	if(argc<2){
		fprintf(stderr,"Usage:......");
		exit(1);
	}
	tbf = mytbf_init(CPS,BURST);
	if(tbf==NULL){
		fprintf(stderr,"mytbf_init() failed!\n");
		exit(1);
	}
	sfd=open(argv[1],O_RDONLY);
	if(sfd<0){
		perror("open():");
		exit(1);
	}
	while(1){

		size = mytbf_fetchtoken(tbf,BUFFSIZE);
		if(size<0)
		{
			fprintf(stderr,"mytbf_fetchtoken():%s\n",strerror(-size));
			exit(1);
		}
		while((len=read(sfd,buf,size))<0)
		{
			if(errno==EINTR) continue;
			perror("read()");
			break;
		}
		if(len==0)break;
		if(size-len>0)
			mytbf_returntoken(tbf,size-len);
		ret=write(dfd,buf,len);
		if(ret<0){
			perror("write():");
			break;
		}
	}
	close(sfd);
	mytbf_destory(tbf);
	exit(1);
}

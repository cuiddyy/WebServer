#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#define BUFFSIZE 1024

int main(int argc,char **argv){
	int sfd,dfd;
	ssize_t len,ret;
	char buf[BUFFSIZE];
	if(argc<3){
		fprintf(stderr,"Usage:......");
		exit(1);
	}
	sfd=open(argv[1],O_RDONLY);
	if(sfd<0){
		perror("open():");
		exit(1);
	}
	dfd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0600);
	if(dfd<0){
		close(sfd);
		perror("open():");
		exit(1);
	}
	while(1){
		len=read(sfd,buf,sizeof(buf));
		if(len<0){
			perror("read()");
			break;
		}
		if(len==0)break;
		ret=write(dfd,buf,len);
		if(ret<0){
			perror("write():");
			break;
		}
	}
	close(sfd);
	close(dfd);
	exit(1);
}

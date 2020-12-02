#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<sys/time.h>
#define CPS 10
#define BUFFSIZE CPS

static volatile int loop = 0;

static void alarm_handler(int s){
	//alarm(1);
	loop=1;
}
int main(int argc,char **argv){
	int sfd,dfd=1;
	ssize_t len,ret;
	char buf[BUFFSIZE];
	struct itimerval itv;
	if(argc<2){
		fprintf(stderr,"Usage:......");
		exit(1);
	}

	
	signal(SIGALRM,alarm_handler);
	//alarm(1);
	
	itv.it_interval.tv_sec=1;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec=1;
	itv.it_value.tv_usec=0;
	if(setitimer(ITIMER_REAL,&itv,NULL)<0)
	{
		perror("setitimer()");
		exit(1);
	}
	

	sfd=open(argv[1],O_RDONLY);
	if(sfd<0){
		perror("open():");
		exit(1);
	}
	while(1){
		while(!loop)
			pause();
		loop=0;
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
	exit(1);
}

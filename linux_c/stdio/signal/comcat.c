#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define CPS 10
#define BUFFSIZE CPS
#define BURST 100
static volatile sig_atomic_t token = 0;

static void alarm_handler(int s){
	alarm(1);
	token++;
	if(token>BURST)
		token=BURST;
}
int main(int argc,char **argv){
	int sfd,dfd=1;
	ssize_t len,ret;
	char buf[BUFFSIZE];
	if(argc<2){
		fprintf(stderr,"Usage:......");
		exit(1);
	}

	
	signal(SIGALRM,alarm_handler);
	alarm(1);
	sfd=open(argv[1],O_RDONLY);
	if(sfd<0){
		perror("open():");
		exit(1);
	}
	while(1){
		while(token<=0)
			pause();
		token--;
		while((len=read(sfd,buf,sizeof(buf)))<0)
		{
			if(errno==EINTR) continue;
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

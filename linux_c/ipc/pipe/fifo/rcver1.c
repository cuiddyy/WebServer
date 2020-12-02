#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<string.h>
#include<errno.h>

#include "fifo.h"


int main(){

	int fd,rlen,wlen;
	char buf[BUFFSIZE];
	fd = mkfifo(FIFOPATH,O_CREAT|O_RDWR);
	if(fd<0){
		if(errno!=EEXIST){
			perror("mkfifo()");
			exit(1);
		}
	}
	fd=open(FIFOPATH,O_RDONLY);	
	while(1){
		rlen = read(fd,buf,BUFFSIZE);
		if(rlen<0){
			perror("write()");
			exit(1);
		}else if(rlen==0){
			printf("write over\n");
		}else{
			wlen = write(STDOUT_FILENO,buf,rlen);
			if(wlen!=rlen){
				perror("write error");
			}
		}
	
	}
	

	exit(0);
}

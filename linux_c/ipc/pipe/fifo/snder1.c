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
	fd = open(FIFOPATH,O_WRONLY);	
	while(1){
		rlen=read(STDIN_FILENO,buf,BUFFSIZE);
		if(rlen<0){
			perror("read()");
			exit(1);
		}
		else if(rlen == 0){
			printf("write over!");
			exit(0);
		}
		else{
			wlen = write(fd,buf,rlen);
			if(wlen != rlen){
				perror("write()");
				exit(1);
			}
		}
	}


	exit(0);
}

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define BUFFSIZE 1024

int main(){
	int pd[2],len;
	pid_t pid;
	char buf[BUFFSIZE];
	if(pipe(pd)<0){
		perror("pipe");
		exit(1);
	}


	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(1);
	}

	if(pid==0)          //child read
	{	close(pd[1]);	//关闭写
		dunp2(pd[0],0);
		close(pd[0]);
		fd = open("/tmp/out",O_RDWR);
		dup2(fd,1);
		dup2(fd,2);
		execl("/usr/local/bin/mpg123","mpg123","-",NULL);
		perror("execl()");
		exit(1);
	
	}else{				//parent write
		close(pd[0]);  //关闭读端
		//父进程从网上收数据，往管道里写
		
		close(pd[1]);
		wait(NULL);
		exit(0);
	
	}


	exit(0);
}


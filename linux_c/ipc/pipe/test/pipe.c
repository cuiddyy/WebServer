#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define DATASIZE 1024

int main(){

	int pd[2],len;
	pid_t pid;
	char data[DATASIZE];
	if(pipe(pd)<0){
		perror("pipe()");
		exit(1);
	}
	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(1);
	}
	if(pid==0){
		close(pd[1]);
		len=read(pd[0],data,DATASIZE);
		write(1,data,len);
		close(pd[0]);
		exit(0);
	}else{
		close(pd[0]);
		write(pd[1],"Hello\n",6);
		close(pd[1]);
		wait(pid);
	}

	exit(0);


}

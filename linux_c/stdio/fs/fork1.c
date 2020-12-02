#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>


int main(){

	pid_t pid;
	printf("[%d]:Begin!\n",getpid());
	fflush(NULL);//这里在fork前要刷新所有的流数据，否则如果输出发生文件重定向，./fork1 > /tmp/out 因为文件流是全缓冲模式，所以如果不刷新缓冲区，父进程缓冲区中的数据会复制到子进程中，那么就会造成fork()前的语句被运行两次。
	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(0);
	}
	if(pid==0){
	
		printf("[%d]:child is working!\n",getpid());
	}else{
		printf("[%d]:parent is working!\n",getpid());
		
	}
	printf("[%d]:End!\n",getpid());
	getchar();
	exit(0);



}

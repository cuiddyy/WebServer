#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(){
	pid_t pid;

	//fflush(NULL);如果这里没有刷新缓冲区，那么父进程中的输出缓冲区会一直保存"Begin()!",在fork一个子进程之后，虽然子进程也复制了父进程的输出缓冲区内容，但是立即就被date替换了，因此子进程输出只有date的输出。在子进程输出完毕后，父进程输出End，如果前面没有刷新缓冲区，那么就会一同输出Begin和End
	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(1);
	}
	if(pid==0){
		execl("/bin/bash","sh","-c","/bin/date","date+%s",NULL);
		perror("execl()");
		exit(1);
	}
	wait(NULL);
	puts("End!");
	exit(0);



}

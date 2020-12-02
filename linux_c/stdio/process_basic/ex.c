#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>



int main(){

	puts("Begin()!");
	fflush(NULL);
	//此处必须加fflush刷新缓冲区
	execl("/bin/date","date","+%s",NULL);
	perror("execl()");
	exit(1);

	puts("End()");
	exit(0);



}

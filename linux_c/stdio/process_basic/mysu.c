#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>



int main(int argc,char **argv){
	int pid;
	if(argc<3){
	
		fprintf(stderr,"Usage....\n");
		exit(1);
	}
	pid = fork();
	if(pid<0){
		perror("fork()");
	}
	if(pid==0){
		setuid(argv[1]);
		execvp(argv[2],argv+2);
	}
	exit(0);
}

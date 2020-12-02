#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#define LEFT  30000000
#define RIGHT 30000200
#define N 3
int main(){
	pid_t pid;
	int i,j,mark,n;
	for(n=0;n<N;n++){
			pid=fork();
			if(pid<0){
				perror("fork()");
				exit(0);
			}
			if(pid==0){
					for(i=LEFT+n;i<=RIGHT;i+=N){
							mark=1;
							for(j=2;j< i/2;j++){
									if(i%j==0){
											mark=0;
											break;
									}
							}	
							if(mark)
									printf("[%d]%d is a primer!\n",n+1,i);
					}
					exit(1);
			}		
	}
	for(n=0;n<N;n++){
		wait(NULL);
	}

	exit(0);


}

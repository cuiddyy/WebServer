#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

#define MYRTSIG (SIGRTMIN+6)
void mysig_handler(int s){

	write(1,"!",1);
}

int main(){
	int i,j;
	sigset_t set,oset,saveset;

	signal(MYRTSIG,mysig_handler);
	sigemptyset(&set);
	sigaddset(&set,MYRTSIG);
	sigprocmask(SIG_UNBLOCK,&set,&saveset);
	sigprocmask(SIG_BLOCK,&set,&oset);
	for(i=0;i<1000;i++){

		for(j=0;j<5;j++){
			write(1,"*",1);
			sleep(1);
		}
		write(1,"\n",1);
		sigsuspend(&oset);
	}
	sigprocmask(SIG_SETMASK,&saveset,NULL);



	exit(0);
}

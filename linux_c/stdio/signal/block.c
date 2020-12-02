#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

void int_handler(int s){

	write(1,"!",1);
}

int main(){
	int i,j;
	sigset_t set,oset,saveset;

	signal(SIGINT,int_handler);
	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	sigprocmask(SIG_UNBLOCK,&set,&saveset);
	for(i=0;i<1000;i++){

		//sigprocmask(SIG_BLOCK,&set,NULL);
		sigprocmask(SIG_BLOCK,&set,&oset);
		for(j=0;j<5;j++){
			write(1,"*",1);
			sleep(1);
		}
		write(1,"\n",1);
		//sigprocmask(SIG_UNBLOCK,&set,NULL);
		sigprocmask(SIG_SETMASK,&oset,NULL);
	}
	sigprocmask(SIG_SETMASK,&saveset,NULL);



	exit(0);
}

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#define LEFT  30000000
#define RIGHT 30000200
#define THRNUM (RIGHT-LEFT+1)



static void *thr_prime(void *p){
	int i,j,mark;
	i = (int)p;
	mark=1;
	for(j=2;j< i/2;j++){
		if(i%j==0){
			mark=0;
			break;
		}
	}
	//printf("mark=%d",mark);
	if(mark)
		printf("%d is a primer!\n",i);
	
	pthread_exit(NULL);
	
}
int main(){
	int i;
	int err;
	pthread_t tid[THRNUM];
	for(i=LEFT;i<=RIGHT;i++){
		err = pthread_create(tid+(i-LEFT),NULL,thr_prime,(void *)i);
		if(err){
			fprintf(stderr,"create failed!%s",strerror(err));
			exit(1);
		}
	}
	for(int i=LEFT;i<=RIGHT;i++){
		pthread_join(tid[i-LEFT],NULL);
	}

	exit(0);


}

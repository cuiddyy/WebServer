#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#define LEFT  30000000
#define RIGHT 30000200
#define THRNUM 4
struct tool{
	int n;
};
static int num = 0;
static pthread_mutex_t mut_num = PTHREAD_MUTEX_INITIALIZER;
static void *thr_prime(void *p){
	int i,j,mark;
	//puts("33");
	while(1){
			pthread_mutex_lock(&mut_num);
			while(num == 0){
				pthread_mutex_unlock(&mut_num);
				sched_yield();
				pthread_mutex_lock(&mut_num);
			}
			if(num == -1){
				pthread_mutex_unlock(&mut_num);
				break;
			}
			i = num;
			num=0;
			pthread_mutex_unlock(&mut_num);
			mark=1;
			for(j=2;j< i/2;j++){
				if(i%j==0){
					mark=0;
					break;
				}
			}
	
			//printf("mark=%d",mark);
		if(mark)
			printf("[%d]%d is a primer!\n",((struct tool*)p)->n,i);
	}
	pthread_exit(NULL);
	
}
int main(){
	int i;
	int err;
	pthread_t tid[THRNUM];
	struct tool *numt;
	//puts("00");
	for(i=0;i<THRNUM;i++){
		numt=malloc(sizeof(*numt));
		err = pthread_create(tid+i,NULL,thr_prime,numt);
		if(err){
			fprintf(stderr,"create failed!%s",strerror(err));
			exit(1);
		}
	}
	//puts("11");
	for(i=LEFT;i<=RIGHT;i++){
		pthread_mutex_lock(&mut_num);

		while(num!=0){
			pthread_mutex_unlock(&mut_num);
			sched_yield();
			pthread_mutex_lock(&mut_num);
		}
		num=i;
		pthread_mutex_unlock(&mut_num);
	
	}
	//puts("33");
	pthread_mutex_lock(&mut_num);
	while(num != 0){
		pthread_mutex_unlock(&mut_num);
		sched_yield();
		pthread_mutex_lock(&mut_num);
	}
	num = -1;
	pthread_mutex_unlock(&mut_num);
	for(int i=0;i<THRNUM;i++){
		pthread_join(tid[i],NULL);
	}
	
	pthread_mutex_destroy(&mut_num);
	exit(0);


}

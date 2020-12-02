#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<errno.h>
#include<pthread.h>
#include "mytbf.h"

#define MYTBF_MAX 1024

struct mytbf_t{
	int cps;
	int burst;
	int token;
	int pos;
	pthread_mutex_t mut;
};

static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_t tid_alrm;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static struct mytbf_t* job[MYTBF_MAX];
typedef void(*sighandler_t)(int);
static int inited = 0;
static sighandler_t alarm_handler_save;

int mytbf_destroy(struct mytbf_t *ptr)
{	struct mytbf_t *me = ptr;
	pthread_mutex_lock(&mut_job);
	job[me->pos]=NULL;
	pthread_mutex_unlock(&mut_job);
	pthread_mutex_destroy(&me->mut);
	free(ptr);

	return 0;
}

static void *thr_alrm(void *s){
	int i;
	//alarm(1);
	while(1)
	{	
		pthread_mutex_lock(&mut_job);
		for(i = 0;i<MYTBF_MAX;i++){
			if(job[i]!=NULL){
				pthread_mutex_lock(&job[i]->mut);
				job[i]->token += job[i]->cps;
				if(job[i]->token > job[i]->burst)
					job[i]->token = job[i]->burst;
				pthread_mutex_unlock(&job[i]->mut);
			}
		}
		pthread_mutex_unlock(&mut_job);
		sleep(1);
	}
}
static void module_unload(void){

	int i;
	//signal(SIGALRM,alarm_handler_save);
	//alarm(0);
	pthread_cancel(tid_alrm);
	pthread_join(tid_alrm,NULL);

	for(i=0;i<MYTBF_MAX;i++){
		if(job[i]!=NULL){
			mytbf_destroy(job[i]);
		}
	}
	pthread_mutex_destroy(&mut_job);
}
static void module_load(void){
	//alarm_handler_save = signal(SIGALRM,alarm_handler);
	//alarm(1);
	int err;
	
	err = pthread_create(&tid_alrm,NULL,thr_alrm,NULL);
	if(err!=0){
		fprintf(stderr,"pthread_create():%d\n",strerror(err));
		exit(1);
	}
	atexit(module_unload);

}
static int get_free_pos_unlocked(void){
	int i;
	for(i=0;i<MYTBF_MAX;i++){
		if(job[i]==NULL)
			return i;
	}
	return -1;
}

struct mytbf_t *mytbf_init(int cps,int burst)
{
	struct mytbf_t *me;
	int pos;
/*
	if(!inited)
	{
		module_load();
		inited = 1;
	}
*/
	pthread_once(&init_once,module_load);
		
	me = malloc(sizeof(*me));
	if(me==NULL) return NULL;
	me->token = 0;
	me->cps = cps;
	me->burst = burst;
	pthread_mutex_init(&me->mut,NULL);
	pthread_mutex_lock(&mut_job);
	pos = get_free_pos_unlocked();
	if(pos<0){
		pthread_mutex_unlock(&mut_job);
		free(me);
		return NULL;
	}
	me->pos = pos;
	job[pos] = me;
	pthread_mutex_unlock(&mut_job);
	return me;

}
static int min(int a,int b){
	return a<b?a:b;
}
int mytbf_fetchtoken(struct mytbf_t *ptr,int size)
{	struct mytbf_t *me = ptr;
	int n;
	if(size<=0)
		return -EINVAL;
	while(me->token<=0){
		pthread_mutex_unlock(&me->mut);
		sched_yield();
		pthread_mutex_lock(&me->mut);
	}
	n = min(me->token,size);

	me->token -= n;
	pthread_mutex_unlock(&me->mut);
	return n;

}
int mytbf_returntoken(struct mytbf_t *ptr,int size)
{	
	struct mytbf_t *me = ptr;
	if(size<=0)
		return -EINVAL;
	
	pthread_mutex_lock(&me->mut);
	me->token += size;
	if(me->token > me->burst)
		me->token = me->burst;
	pthread_mutex_unlock(&me->mut);

	return size;
}

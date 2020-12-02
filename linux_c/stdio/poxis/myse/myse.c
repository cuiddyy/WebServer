#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"myse.h"

struct myse_t{

	int value;
	pthread_mutex_t mut;
	pthread_cond_t cond;
};


myse_t *myse_init(int initval){
	struct myse_t *me;
	me = malloc(sizeof(*me));
	if(me==NULL)
		return NULL;
	me->value = initval;
	pthread_mutex_init(&me->mut,NULL);
	pthread_cond_init(&me->cond,NULL);

	return me;

}

int myse_add(myse_t *ptr,int n){
	struct myse_t *me = ptr;
	pthread_mutex_lock(&me->mut);
	me->value += n;
	pthread_cond_broadcast(&me->cond);
	pthread_mutex_unlock(&me->mut);

}

int myse_sub(myse_t *ptr,int n){
	struct myse_t *me = ptr;
	pthread_mutex_lock(&me->mut);
	while(me->value < n){
		pthread_cond_wait(&me->cond,&me->mut);
	}
	me->value -= n;
	pthread_mutex_unlock(&me->mut);
	return n;
}

int myse_destroy(myse_t *ptr){
	struct myse_t *me = ptr;
	pthread_mutex_destroy(&me->mut);
	pthread_cond_destroy(&me->cond);
	free(me);
}

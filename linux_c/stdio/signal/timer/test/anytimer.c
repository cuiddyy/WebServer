#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<errno.h>
#include<sys/time.h>

#include "anytimer.h"





enum{
	STATE_RUNNING=1,
	STATE_CANCELED,
	STATE_OVER

};



struct at_job_st{

	int state;
	int sec;
	int time_remain;
	int repeat;
	at_jobfunc_t *jobp;
	void *arg;
};


static struct sigaction at_sa_save;
static struct at_job_st *job[JOB_MAX];
static int inited=0;

static int get_free_pos(void){
	int i;
	for(i=0;i<JOB_MAX;i++){
		if(job[i]==NULL) return i;
	}
	return -1;

}

static void at_handler(int s,siginfo_t *infop,void *unused){
	int i;
	if(infop->si_code != SI_KERNEL)
		return;
	for(i=0;i<JOB_MAX;i++){
		if(job[i]!=NULL && job[i]->state == STATE_RUNNING){
			job[i]->time_remain --;
			if(job[i]->time_remain==0){
				job[i]->jobp(job[i]->arg);
				if(job[i]->repeat==1){
				 	job[i]->time_remain = job[i]->sec;	
				}else{
					job[i]->state=STATE_OVER;
				}
			}
		}
	
	}

}

static void module_unload(void){
	struct itimerval itv;

	itv.it_interval.tv_sec=0;
	itv.it_interval.tv_usec=0;
	itv.it_value.tv_sec=0;
	itv.it_value.tv_usec=0;
	if(setitimer(ITIMER_REAL,&itv,NULL)<0){
		perror("setitimer()");
		exit(1);
	}
	if(sigaction(SIGALRM,&at_sa_save,NULL)<0){
		perror("sigaction()");
		exit(1);
	}
}


static void module_load(void){
	struct itimerval itv;
	struct sigaction sa;
	itv.it_interval.tv_sec=1;
	itv.it_interval.tv_usec=0;
	itv.it_value.tv_sec=1;
	itv.it_value.tv_usec=0;
	if(setitimer(ITIMER_REAL,&itv,NULL)<0){
		perror("setitimer()");
		exit(1);
	}
	sa.sa_sigaction = at_handler;
	sa.sa_flags=SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM,&sa,&at_sa_save);
	
	atexit(module_unload);

}

int at_addjob(int sec,at_jobfunc_t *jobp,void *arg){
	int pos;
	struct at_job_st *me;
	if(!inited){
		module_load();
		inited = !inited;
	}
	pos = get_free_pos();
	if(pos<0){
		return -ENOSPC;
	}
	me = malloc(sizeof(*me));
	if(me == NULL){
		return -ENOMEM;
	}

	me->state = STATE_RUNNING;
	me->sec = sec;
	me->time_remain = sec;
	me->jobp = jobp;
	me->arg = arg;

	job[pos] = me;

	return pos;
}
int at_addjob_repeat(int sec,at_jobfunc_t *jobp,void *arg){
	int pos;
	struct at_job_st *me;
	if(!inited){
		module_load();
		inited = !inited;
	}
	pos = get_free_pos();
	if(pos<0){
		return -ENOSPC;
	}
	me = malloc(sizeof(*me));

	me->state = STATE_RUNNING;
	me->sec = sec;
	me->time_remain = sec;
	me->repeat = 1;
	me->jobp = jobp;
	me->arg = arg;

	job[pos] = me;

	return pos;
}

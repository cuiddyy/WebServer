#ifndef ANYTIMER_H_
#define ANYTIMER_H_

#define JOB_MAX 1024

typedef void at_jobfunc_t(void*);

int at_addjob(int sec,at_jobfunc_t *jobp,void *arg);

int at_addjob_repeat(int sec,at_jobfunc_t *jobp,void *arg);

int at_canceljob(int id);

int at_waitjob(int id); 






#endif

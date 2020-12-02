#ifndef MYSE_H_
#define MYSE_H_


typedef void myse_t;

myse_t *myse_init(int initval);

int myse_add(myse_t *,int);

int myse_sub(myse_t *,int);

int myse_destroy(myse_t *);
 




#endif

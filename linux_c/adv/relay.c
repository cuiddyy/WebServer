#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/select.h>



#define TTY1 "/tmp/tty11"
#define TTY2 "/tmp/tty12"
#define BUFSIZE 1024
enum{
	STATE_R=1,
	STATE_W,
STATE_AUTO,
	STATE_Ex,
	STATE_T

};
struct fsm_st{

	int state;
	int sfd;
	int dfd;
	int len;
	int pos;
	char *errstr;
	char buf[BUFSIZE];
};

static void fsm_driver(struct fsm_st *fsm){
		int ret;
		//printf("%d\n",fsm->state);
		switch(fsm->state){
	
		case STATE_R:
			fsm->len = read(fsm->sfd,fsm->buf,BUFSIZE);
			lseek(fsm->sfd,0,SEEK_SET);
			if(fsm->len==0){
				puts("rttt");
				fsm->state = STATE_T;
			}else if(fsm->len<0){
				if(errno==EAGAIN)
					fsm->state = STATE_R;
				else{
					fsm->errstr="read()";
					fsm->state = STATE_Ex;
				}
			}else{
				fsm->pos = 0;
				fsm->state = STATE_W;
			}
			break;
		case STATE_W:
			ret = write(fsm->dfd,fsm->buf+fsm->pos,fsm->len);//
			if(ret < 0){
				if(errno==EAGAIN)
					fsm->state = STATE_W;
				else{
					fsm->errstr="write()";
					fsm->state = STATE_Ex;
				}
			}else{
				fsm->pos += ret;
				fsm->len -= ret;
				if(fsm->len==0)
					fsm->state = STATE_R;
				else
					fsm->state = STATE_W;
			}
			break;
		case STATE_Ex:
			//puts("eeee");
			perror(fsm->errstr);
			fsm->state = STATE_T;
			break;
		case STATE_T:
			puts("tttt");
			/**
			 *do sth
			 */
			break;
		default:
			/**
			 *do sth
			 */
			abort();
			break;
	}
}

static int max(int a,int b){
	return a>b?a:b;
}
static void relay(int fd1,int fd2){
	int fd1_save,fd2_save;
	struct fsm_st fsm12,fsm21;
	fd_set rset,wset;

	fd1_save = fcntl(fd1,F_GETFL);
	fcntl(fd1,F_SETFL,fd1_save|O_NONBLOCK);

	fd2_save=fcntl(fd2,F_GETFL);
	fcntl(fd2,F_SETFL,fd2_save|O_NONBLOCK);

	fsm12.state = STATE_R;
	fsm12.sfd=fd1;
	fsm12.dfd=fd2;

	fsm21.state = STATE_R;
	fsm21.sfd=fd2;
	fsm21.dfd=fd1;
	
	while(fsm12.state != STATE_T || fsm21.state !=STATE_T){
		//布置监视任务***************************************重点在这里****************************
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if(fsm12.state==STATE_R)
			FD_SET(fsm12.sfd,&rset);
		if(fsm12.state==STATE_W)
			FD_SET(fsm12.dsd,&wset);
		if(fsm21.state == STATE_R)
			FD_SET(fsm21.sfd,&rset);
		if(fsm21.state == STATE_W)
			FD_SET(fsm21.dfd,&wset);
		//监视
		if(fsm12.state<STATE_AUTO || fsm21.state<STATE_AUTO){
				if(select(max(fd1,fd2)+1,&rset,&wset,NULL,NULL)<0){
					if(errno ==EINTR)
						continue;
					perror("select()");
					exit(1);
				
			}
		}
		//查看监视列表
		if(FD_ISSET(fd1,&rset) || FD_ISSET(fd2,&wset) || fsm12.state > STATE_AUTO)
			fsm_driver(&fsm12);
		if(FD_ISSET(fd2,&rset) || FD_ISSET(fd1,&wset) || fsm21.state > STATE_AUTO)
			fsm_driver(&fsm21);

		//****************************************************************************	
		//printf("%d\n",fsm12.state);
		//printf("%d\n",fsm21.state);
		//printf("*********************\n");
	}
	fcntl(fd1,F_SETFL,fd1_save);
	fcntl(fd2,F_SETFL,fd2_save);
}


int main(){
	int fd1,fd2;
	fd1=open(TTY1,O_RDWR);
	if(fd1<0){
		perror("fd1 open()");
		exit(1);
	}
	write(fd1,"TTY1\n",5);

	fd2 = open(TTY2,O_RDWR|O_NONBLOCK);
	if(fd2<0){
		perror("fd2 open()");
		exit(1);
	}
	write(fd2,"TTY\n",5);
	relay(fd1,fd2);



	close(fd1);
	close(fd2);
	exit(0);
}

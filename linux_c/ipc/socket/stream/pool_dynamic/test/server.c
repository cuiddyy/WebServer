#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<sys/mman.h>


#include "proto.h"

#define SIG_NOTIFY      SIGUSR2
#define MAXCLIENTS     20
#define MAXSPARESERVER 10
#define MINSPARESERVER 5
#define IPSTRSIZE      40
#define LINEBUFSIZE    80

enum{
	STATE_IDLE=0,
	STATE_BUSY
};

struct server_st{
	pid_t pid;
	int state;
};

static int sd;
static struct server_st *serverpool;
static int idle_cnt=0,busy_cnt=0;
static void usr2_handler(){
	return;
}
static void server_job(int pos){
	int client_sd,len;
	struct sockaddr_in raddr;
	socklen_t raddr_len;
	int ppid;
	char ipstr[IPSTRSIZE];
	char linebuf[LINEBUFSIZE];
	time_t stamp;

	ppid=getppid();
	//printf("server_job----pid=%d  ppid=%d\n",getpid(),ppid);
	while(1){
		serverpool[pos].state = STATE_IDLE;
		kill(ppid,SIG_NOTIFY);
		client_sd = accept(sd,(void *)&raddr,&raddr_len);
		if(client_sd<0){
			if(errno!=EINTR || errno !=EAGAIN){
				perror("accept()");
				exit(1);
			}
		}
		//printf("while\n");
		serverpool[pos].state = STATE_BUSY;
		kill(ppid,SIG_NOTIFY);
		inet_ntop(AF_INET,&raddr.sin_addr,ipstr,IPSTRSIZE);
		stamp=time(NULL);
		len = snprintf(linebuf,LINEBUFSIZE,FMT_STAMP,stamp);
		send(client_sd,linebuf,len,0);
		sleep(5);
		close(client_sd);
	}
}
static int add_1_server(){
	int slot,j;
	pid_t pid;

	//printf("self=%d\n",getpid());
	//for(j=0;j<MAXCLIENTS;j++){
	//	printf("%d pid is %d\n",j,serverpool[j].pid);
	//}
	if(idle_cnt+busy_cnt >= MAXCLIENTS)
			return -1;
	for(slot=0;slot<MAXCLIENTS;slot++){
		if(serverpool[slot].pid==-1){

			break;
		}
	}
	
	serverpool[slot].state=STATE_IDLE;
	//puts("add");
	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(1);
	}
	//printf("add-%d\n",pid);
	if(pid==0){
		server_job(slot);
		exit(0);
	}else{
		//printf("seltf=%d---else-pid=%d\n",getpid(),pid);

		serverpool[slot].pid=pid;
		//printf("--pid=%d\n",serverpool[slot].pid);
		idle_cnt++;
	}
	//printf("end\n");
	return 0;
}
static int add_2_server(){

	

}
static int del_1_server(){
	int i;
	if(idle_cnt==0) return -1;
	for(i=0;i<MAXCLIENTS;i++){
		if(serverpool[i].pid!=-1 && serverpool[i].state==STATE_IDLE){
			kill(serverpool[i].pid,SIGTERM);
			serverpool[i].pid=-1;
			idle_cnt--;
			break;
		}
	}
	return 0;

}

static int scan_pool(){
	int i,busy=0,idle=0;

	for(i=0;i<MAXCLIENTS;i++){
		//printf("pid=%d\n",serverpool[i].pid);
		if(serverpool[i].pid==-1) continue;
		if(kill(serverpool[i].pid,0)){
			serverpool[i].pid=-1;
			continue;
		}

		if(serverpool[i].state == STATE_IDLE){
			idle++;
		}else if(serverpool[i].state == STATE_BUSY){
			busy++;
		}else{
			fprintf(stderr,"Unknown\n");
			exit(1);
		}
	}
	idle_cnt=idle;
	busy_cnt=busy;
	return 0;

}

int main(){

	int i;
	struct sockaddr_in laddr;
	struct sigaction sa,osa;
	sigset_t set,oset;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD,&sa,&osa);

	sa.sa_handler = usr2_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIG_NOTIFY,&sa,&osa);

	sigemptyset(&set);
	sigaddset(&set,SIG_NOTIFY);
	sigprocmask(SIG_BLOCK,&set,&oset);
//	puts("111");
	serverpool = mmap(NULL,sizeof(struct server_st)*MAXCLIENTS,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
	if(serverpool == MAP_FAILED){
		perror("mmap()");
		exit(0);
	}
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd<0){
		perror("socket()");
		exit(0);
	}
//	puts("222");
	int val=1;
	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0){
		perror("setsockopt()");
		exit(0);
	}

	laddr.sin_family=AF_INET;
	laddr.sin_port=htons(atoi(SERVERPORT));	
	inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
	//puts("22");
	if(bind(sd,(void *)&laddr,sizeof(laddr))<0){
		perror("bind()");
		exit(1);
	}
	
	if(listen(sd,100)<0){
		perror("listen()");
		exit(1);
	}


	for(i=0;i<MAXCLIENTS;i++){
		serverpool[i].pid=-1;
	}

	for(i=0;i<MINSPARESERVER;i++){
		//scan_pool();
		add_1_server();
		
		//printf("self=%d\n",getpid());
		//scan_pool();
	}
	//printf("pre-cnt=%d\n",idle_cnt);
	while(1){

		sigsuspend(&oset);
		
		scan_pool();
		
		//printf("cnt=%d\n",idle_cnt);
		if(idle_cnt<MINSPARESERVER){
			for(i=0;i<(MINSPARESERVER-idle_cnt);i++){
				add_1_server();
			}
		}else if(idle_cnt>MAXSPARESERVER){
			for(i=0;i<(idle_cnt-MAXSPARESERVER);i++){
				del_1_server();
			}
		}
		
		for(i=0;i<MAXCLIENTS;i++){
			if(serverpool[i].pid==-1){
				putchar(' ');
			}else if(serverpool[i].state == STATE_IDLE){
				putchar('.');
			}else{
				putchar('X');
			}
		}
		putchar('\n');
	}
	
	sigprocmask(SIG_SETMASK,&oset,NULL);

	exit(0);
}

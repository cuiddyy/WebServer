#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<sys/mman.h>
#include "proto.h"

#define MINSPARESERVER 5
#define MAXSPARESERVER 10
#define MAXCLIENTS         20
#define SIG_NOTIFY     SIGUSR2
#define IPSTRSIZE      40
#define LINEBUFSIZE    80

enum{
	STATE_IDLE=0,
	STATE_BUSY

};

static struct server_st *serverpool;
static int idle_count = 0,busy_count = 0;
static int sd;

struct server_st{
	pid_t pid;
	int state;

};
static void usr2_handler(){
	return ;
}

static void server_job(int pos){
	int ppid;
	struct sockaddr_in raddr;
	socklen_t raddr_len;
	ppid = getppid();
	int client_sd;
	char ipstr[IPSTRSIZE];
	char linebuf[LINEBUFSIZE];
	time_t stamp;
	int len;
	while(1){
		serverpool[pos].state = STATE_IDLE;
		kill(ppid,SIG_NOTIFY);
		client_sd = accept(sd,(void *)&raddr,&raddr_len);
		if(client_sd < 0){
			if(errno != EINTR || errno != EAGAIN){
				perror("accept()");
				exit(1);
			}
		}
			
		serverpool[pos].state = STATE_BUSY;
		kill(ppid,SIG_NOTIFY);
		inet_ntop(AF_INET,&raddr.sin_addr,ipstr,IPSTRSIZE);
		//printf("[%d]client:%s:%d\n",getpid,ipstr,ntohs(raddr.sin_port));
		stamp = time(NULL);
		len = snprintf(linebuf,LINEBUFSIZE,FMT_STAMP,stamp);
		send(client_sd,linebuf,len,0);
		sleep(5);
		close(client_sd);
	}
}


static int add_1_server(){
	int slot,i;
	pid_t pid;
	if(idle_count + busy_count >= MAXCLIENTS)
		return -1;
	printf("self=%d\n",getpid());
	for(i=0;i<MAXCLIENTS;i++){
		printf("pid=%d\n",serverpool[i].pid);
	}
	for(slot=0;slot<MAXCLIENTS;slot++){
		if(serverpool[slot].pid == -1)
			break;
	}
	serverpool[slot].state = STATE_IDLE;
	pid = fork();
	if(pid<0){
		perror("fork()");
		exit(1);
	}
	//puts("add");
	if(pid==0){
		server_job(slot);
		exit(0);
	}else{
		serverpool[slot].pid=pid;
		idle_count++;
	}
	return 0;



}
static int del_1_server(){
	int i;
	if(idle_count == 0)
		return -1;
	for(i=0;i<MAXCLIENTS;i++){
		if(serverpool[i].pid!=-1 && serverpool[i].state == STATE_IDLE){
			kill(serverpool[i].pid,SIGTERM);//杀死进程
			serverpool[i].pid = -1;
			idle_count--;
			break;
		}
	}
	return 0;

}
static int scan_pool(){
	int i,busy = 0,idle = 0;
	for(i=0;i<MAXCLIENTS;i++){
		//printf("pid=%d\n",serverpool[i].pid);
		if(serverpool[i].pid == -1)
			continue;
		if(kill(serverpool[i].pid,0)){//0用来检测进程否还存在
			serverpool[i].pid = -1;//如果不存在
			continue;
		}
		if(serverpool[i].state == STATE_IDLE)
			idle++;
		else if(serverpool[i].state == STATE_BUSY)
			busy++;
		else {
			fprintf(stderr,"Unkonwn state.\n");
			abort();
		}
	}
	idle_count = idle;
	busy_count=busy;
	return 0;
}
int main(){
	int i;
	struct sigaction sa,osa;
	struct sockaddr_in laddr;
	sigset_t set,oset;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);//这里的信号处理函数是忽略，所以用不到sa.mask这个信号集来屏蔽信号，所以可以直接置空。
	sa.sa_flags = SA_NOCLDWAIT;//阻止子进程成为僵尸状态，子进程自行消亡。
	sigaction(SIGCHLD,&sa,&osa);//父进程就是通过SIGCHLD这个信号来通知wait对子进程进行收尸的，因为现在要让子进程自己消亡，所以就需要让父进程忽略SIGCHLD信号。
	
	sa.sa_handler = usr2_handler;
	sigemptyset(&sa.sa_mask);//没有同时使用同一个信号处理函数的情况，所以可以不必屏蔽信号
	sa.sa_flags = 0;
	sigaction(SIG_NOTIFY,&sa,&osa);
	
	sigemptyset(&set);
	sigaddset(&set,SIG_NOTIFY);
	sigprocmask(SIG_BLOCK,&set,&oset);

	serverpool = mmap(NULL,sizeof(struct server_st)*MAXCLIENTS,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);//因为MAP_ANONYMOUS设置是使用匿名地址空间，所以后面的文件描述符可以不写，填-1.
	if(serverpool == MAP_FAILED){
		perror("mmap()");
		exit(1);
	}
	for(i=0;i<MAXCLIENTS;i++){
		serverpool[i].pid = -1;
	}
	sd = socket(AF_INET,SOCK_STREAM,0);
	if(sd<0){
		perror("sock()");
		exit(1);
	}
	int val = 1;
	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0){
		perror("setsockopt()");
		exit(1);
	}
	laddr.sin_family = AF_INET;
	laddr.sin_port =htons(atoi(SERVERPORT));
	inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);
	if(bind(sd,(void *)&laddr,sizeof(laddr))<0){
		perror("bind()");
		exit(1);
	
	}

	if(listen(sd,100)<0){
		perror("listen()");
		exit(1);
	}
	
	for(i=0;i<MINSPARESERVER;i++){
		add_1_server();
	
	}

	while(1){
		sigsuspend(&oset);//因为之前已经用sigprocmask将当前进程的mask中SIG_NOTIFY对应位置0,即阻塞了SIG_NOTIFY，所以当前的sigsuspend会将mask重新设置为oste，也就是解除了SIG_NOTIFY信号的屏蔽，这时SIG_NOTIFY被响应，执行完后，sigsuspend()会将屏蔽字恢复为原来屏蔽SIG_NOTIFY信号的模式，因为此时它要统计idle_count，调整进程数量，所以此时进程不能进行socket连接处理。
		scan_pool();
		if(idle_count > MAXSPARESERVER){
			for(i=0;i<(idle_count-MAXSPARESERVER);i++){
				del_1_server();
			}
		}else if(idle_count<MINSPARESERVER){
			for(i=0;i<(MINSPARESERVER-idle_count);i++){
				add_1_server();
			}
		}
		//print status of pool
		for(int i=0;i<MAXCLIENTS;i++){
			if(serverpool[i].pid == -1){
				putchar(' ');
			}else if(serverpool[i].state == STATE_IDLE){
				putchar('.');
			}else{
				putchar('X');
			}
			putchar('\n');
			//fflush(NULL);
		}
	}

	sigprocmask(SIG_SETMASK,&oset,NULL);

	exit(0);
}

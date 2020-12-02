#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<syslog.h>
#include<errno.h>
#include<signal.h>
#define FNAME "/tmp/out"
static FILE *fp;
static int daemonize(){
	pid_t pid;
	int fd;
	pid=fork();
	printf("pid=%d",pid);
	if(pid<0){
		return -1;
	}
	if(pid>0)
		exit(0);
	fd = open("test_daemon",O_RDWR);
	if(fd<0){
		return -1;
	}
	dup2(fd,0);
	dup2(fd,1);
	dup2(fd,2);
	if(fd>2)
		close(fd);
	setsid();
	chdir("/");
	return 0;

}
void daemon_exit(int s){

	fclose(fp);
	closelog();
}
int main(){

	int i;
	struct sigaction sa;
	
	sa.sa_handler = daemon_exit;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask,SIGQUIT);
	sigaddset(&sa.sa_mask,SIGTERM);
	sigaddset(&sa.sa_mask,SIGINT);
	sa.sa_flags = 0;
	sigaction(SIGINT,&sa,NULL);
	sigaction(SIGQUIT,&sa,NULL);
	sigaction(SIGTERM,&sa,NULL);
	//signal(SIGINT,daemon_exit);
	//signal(SIGQUIT,daemon_exit);
	//signal(SIGTERM,daemon_exit);
		
	openlog("mydaemon",LOG_PID,LOG_DAEMON);

	if(daemonize())
	{
		syslog(LOG_ERR,"daemonize() failed!");
		exit(1);
	}else{
		syslog(LOG_INFO,"daemonize() successded!");
	}
	fp=fopen(FNAME,"w");
	if(fp==NULL){
		syslog(LOG_ERR,"fopen():%s",strerror(errno));
		exit(1);
	}else{
		syslog(LOG_INFO,"%s was opened!",FNAME);
	}
	for(i=0;;i++){
		fprintf(fp,"%d\n",i);
		fflush(fp);
		syslog(LOG_DEBUG,"%d is printed.",i);
		sleep(1);
	}
	exit(0);
}

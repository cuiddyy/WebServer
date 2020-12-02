#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<glob.h>
#include<string.h>
#include<sys/wait.h>
#define DELIMS " \t\n"

struct cmd_st{

	glob_t globres;
};
static void prompt(void){

	printf("myshell-0.1$");
}
static void parse(char *line,struct cmd_st *res){
	char *tok;
	int i=0;
	while(1){
		
		tok = strsep(&line,DELIMS);
		printf("tok=%s \n",tok);
		if(tok==NULL)
				break;
		if(tok[0]=='\0')
				continue;
		glob(tok,GLOB_NOCHECK|GLOB_APPEND*i,NULL,&res->globres);
		i=1;
	
	}
}
int main(){
	char *linebuf=NULL;
	size_t linebuf_size=0;
	struct cmd_st res;
	int pid;
	while(1){
		
		prompt();
		if(getline(&linebuf,&linebuf_size,stdin)<0)
				break;
		parse(linebuf,&res);
		if(0){//是内部命令
			
		}else{//是外部命令
			pid = fork();
			if(pid<0){
				perror("fork()!");
				exit(1);
			}
			if(pid == 0){
				puts(res.globres.gl_pathv[1]);
				execvp(res.globres.gl_pathv[0],res.globres.gl_pathv);
				perror("execvp()");
				exit(1);
			}else{
				wait(NULL);
			}
		}
	}



	exit(0);



}

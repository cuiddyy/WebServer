#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<errno.h>
int main(int argc,char **argv){
	FILE *fs=NULL;
	int count=0,ch;
	fs = fopen(argv[1],"r");
	//防止假错误
	do{
		if(fs==NULL){
			if(errno == EINTR)
					continue;
			perror("fs-open:");
			exit(0);	
		}
		
	}while(fs==NULL);
	while(1){
		ch = fgetc(fs);
		if(ch==EOF) break;
		count++;
	}
	fclose(fs);
	printf("chars'num=%d\n",count);
	exit(1);
}

#include<stdio.h>
#include<stdlib.h>

int main(int argc,char **argv){
	FILE *fs=NULL;
	int count=0,ch;
	fs = fopen(argv[1],"r");
	if(fs==NULL){
		perror("fs-open:");
		exit(0);	
	}
	while(1){
		ch = fgetc(fs);
		if(ch==EOF) break;
		count++;
	}
	fclose(fs);
	printf("chars'num=%d\n",count);
	exit(1);
}

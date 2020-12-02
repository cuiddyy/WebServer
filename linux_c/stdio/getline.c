#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main(int argc,char **argv){
	FILE *fs=NULL;
	char *linebuff=NULL;
	size_t linesize=0;
	int len;
	if(argc<2){
		fprintf(stderr,"Usage...\n");
		exit(1);
	}
	fs=fopen(argv[1],"r");
	if(fs==NULL){
		perror("fopen():");
		exit(1);
	}
	while(1){
		len = getline(&linebuff,&linesize,fs);
		if(len<0)
			break;
		printf("linebuff.size=%d\n",strlen(linebuff));
		printf("linesize=%d\n",linesize);
	}
	



	fclose(fs);
	exit(0);
}

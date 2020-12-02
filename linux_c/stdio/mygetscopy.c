#include<stdio.h>
#include<stdlib.h>
#define BUFFSIZE 1024

int main(int argc,char **argv){
	FILE *fs=NULL;
	FILE *fd=NULL;
	char buff[BUFFSIZE];
	if(argc<3){
		fprintf(stderr,"usage is wrong:%s <src_file><dest_file>\n",argv[0]);
		exit(0);
	}
	fs = fopen(argv[1],"r");
	if(fs==NULL){
		perror("fs-open:");
        }
	fd = fopen(argv[2],"w");
	if(fd==NULL){
		fclose(fd);
		perror("fd-open:");
		exit(1);
	}
	while(fgets(buff,BUFFSIZE,fs)!=NULL){
		fputs(buff,fd);
	}
	fclose(fd);
	fclose(fs);
	exit(0);

}

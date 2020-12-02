#include<stdio.h>
#include<stdlib.h>


int main(int argc,char **argv){
	FILE *fs=NULL;
	FILE *fd=NULL;
	int ch;
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
	while(1){
		ch = fgetc(fs);
		if(ch==EOF)
			break;
		fputc(ch,fd);
	}
	fclose(fd);
	fclose(fs);
	exit(0);

}

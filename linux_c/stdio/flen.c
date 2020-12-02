#include<stdio.h>
#include<stdlib.h>



int main(int argc,char **argv){
	FILE *fs=NULL;
	fs = fopen(argv[1],"r");
	if(fs==NULL){
		perror("fopen:");
		exit(0);
	}
	
	fseek(fs,0,SEEK_END);
	printf("file's len =%ld\n",ftell(fs));
	fclose(fs);
	exit(1);
}

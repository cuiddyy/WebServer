#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
static int flen(const char *filename){
	struct stat statres;
	if(stat(filename,&statres)<0){
	
		perror("stat():");
		exit(1);
	}
	return statres.st_size;

}


int main(int argc,char **argv){


	printf("file_size=%d\n",flen(argv[1]));
	exit(0);


}

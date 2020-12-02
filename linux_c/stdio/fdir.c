#include<stdio.h>
#include<stdlib.h>
#define PAT "/etc/"
int main(){

	
	DIR *dp;
	struct dirent *cur;
	dp=opendir(PAT);
	if(dp==NULL){
		perror("opendir()");
		exit(1);
	}


		puts(cur->dname);
	
	}
	exit(0);
}

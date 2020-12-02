#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define FNAME "/tmp/out"
#define BUFFSIZE 1024
int main(){
	struct tm *tm;
	FILE *fs;
	char buf[BUFFSIZE];
	int count=0;
	time_t stamp;
	fs=fopen(FNAME,"a+");
	if(fs==NULL){
		perror("open():");
		exit(1);
	}
	while(fgets(buf,BUFFSIZE,fs)!=NULL){
		count++;
	}
	
	while(1){
	
		time(&stamp);
		tm = localtime(&stamp);
		fprintf(fs,"%-4d%d-%d-%d %d:%d:%d\n",++count,\
				tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,\
				tm->tm_hour,tm->tm_min,tm->tm_sec);
		fflush(fs);
		sleep(1);
	
	}



	fclose(fs);
}

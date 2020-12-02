#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<string.h>
#define TIMESTRSIZE 1024
#define FMTSTRSIZE 1024
/*
 *-y: year
 *-m: month
 *-d: day
 *-H: hour
 *-M:minute
 *-S:second
 * */

int main(int argc,char **argv){
	FILE *fp = stdout;
	struct tm *tm;
	time_t stamp;
	char timestr[TIMESTRSIZE];
	char fmstr[FMTSTRSIZE];
	int c;
	char t='\n';
	fmstr[0]='\0';
	stamp = time(NULL);

	tm = localtime(&stamp);
	printf("t=%d\n",t);
	while(1){
	
		c = getopt(argc,argv,"-H:MSy:md");
		if(c<0) {
			printf("break c=%d\n",c);
			break;
		}
		switch(c){
			case 1:
				fp = fopen(argv[optind-1],"w");
				if(fp==NULL){
					perror("fopen():");
					fp=stdout;
				}
				break;
			case 'H':
				if(strcmp(optarg,"12") == 0)
						strncat(fmstr,"%I(%P) ",FMTSTRSIZE);
				else if(strcmp(optarg,"24") == 0)
						strncat(fmstr,"%H ",FMTSTRSIZE);
				else
						fprintf(stderr,"INvalid argument\n");
				break;
			case 'M':
				strncat(fmstr,"%M ",FMTSTRSIZE);
				break;
			case 'S':
				strncat(fmstr,"%S ",FMTSTRSIZE);
				break;
			case 'y':
				if(strcmp(optarg,"2") == 0)
						strncat(fmstr,"%y ",FMTSTRSIZE);
				else if(strcmp(optarg,"4") == 0)
						strncat(fmstr,"%Y ",FMTSTRSIZE);
				else
						fprintf(stderr,"Invalid argment of -y\n");
				break;
			case 'm':
				strncat(fmstr,"%m ",FMTSTRSIZE);
				break;
			case 'd':
				strncat(fmstr,"%d ",FMTSTRSIZE);
				break;
		
		}
	}
	strncat(fmstr,"\n",FMTSTRSIZE);
	strftime(timestr,TIMESTRSIZE,fmstr,tm);
	fputs(timestr,fp);
	if(fp!=stdout)
			fclose(fp);
	exit(0);

	
}

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int UDPPort;
int DataSize;
int ErrorRate; //如何实现错误和丢失！zhao 
int LostRate; //0-10
int SWSize; //0-10
int InitSeqNo;
int Timeout;
char temp[1024];


void readConfig(const char *addr){
	FILE *fp;
	if((fp = fopen(addr,"r")) == NULL){
		perror("fail to read");
		exit (1);
	}
 
	fscanf(fp,"%s = %d\n",temp,&UDPPort);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&DataSize);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&ErrorRate);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&LostRate);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&SWSize);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&InitSeqNo);
	//printf("%s\n",temp);
	fscanf(fp,"%s = %d\n",temp,&Timeout);
	//printf("%s\n",temp);
}

int isLost(){
	int a = rand()%10;
	if(a < LostRate){
		return 1;
	}else{
		return 0;
	}
}

int isError(){
	srand(time(NULL));
	int a = rand()%10;
	if(a < ErrorRate){
		return 1;
	}else{
		return 0;
	}
}


#endif

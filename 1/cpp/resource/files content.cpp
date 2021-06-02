#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
	FILE *fp;
	fp =fopen("sendtoA.txt","w");
	if(fp == NULL){
		return 0;
	}
	int n = 100000;
	srand(time(0));
	while(n--){
		fprintf(fp,"%c",rand()%60+32);
	}
	fclose(fp);
	return 0;
}

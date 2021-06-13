#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
	FILE *fp;
	fp =fopen("sendtoA.txt","w");
	if(fp == NULL){
		return 0;
	}
	int n = 70000;
	srand(time(0));
	while(n--){
		char a = rand()%60+32;
		if(a == '\0'){
			continue;
		}
		fprintf(fp,"%c",a);
	}
	//fprintf(fp,"EE--EE");
	fclose(fp);
	return 0;
}

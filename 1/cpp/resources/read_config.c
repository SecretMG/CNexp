#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{

 FILE *fp;            /*文件指针*/

  /*写文件*/
//  if( (fp=fopen("configue.txt","wt+")) == NULL ){
//         puts("Fail to open file!");
//         exit(0);
//     }
// fprintf(fp,"%s = %d\n","UDPPort",4000);
// fprintf(fp,"%s = %d\n","DataSize",1024);
// fprintf(fp,"%s = %d\n","ErrorRate",1);
// fprintf(fp,"%s = %d\n","LostRate",1);
// fprintf(fp,"%s = %d\n","SWSize",5);
// fprintf(fp,"%s = %d\n","InitSeqNo",0);
// fprintf(fp,"%s = %d\n","Timeout",1);

/*读文件*/
 if((fp = fopen("D:\\VS\\lab_network\\configue.txt","r")) == NULL)
 {
 perror("fail to read");
 exit (1) ;
 }
 char temp[1024];
 //配置
 int UDPPort;
 int DataSize;
 int ErrorRate;
 int LostRate;
 int SWSize;
 int InitSeqNo;
 int Timeout;
 //fscanf(fp,"%s = %d\n",temp,UDPPort);
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

printf("%s=%d\n","UDPPort",UDPPort);
printf("%s=%d\n","DataSize",DataSize);
printf("%s=%d\n","ErrorRate",ErrorRate);
printf("%s=%d\n","LostRate",LostRate);
printf("%s=%d\n","SWSize",SWSize);
printf("%s=%d\n","InitSeqNo",InitSeqNo);
printf("%s=%d\n","Timeout",Timeout);
fclose(fp);
}

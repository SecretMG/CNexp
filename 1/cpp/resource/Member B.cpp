//客户端1
//#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "frame.h" 
#include "settings.h"
#include "GBN.h"
#pragma comment(lib,"WS2_32.lib")

#define FRAMESIZE 3000 //自己定义,缓冲区大小（每次缓冲区内存放一个帧） 
#define DATASIZE 1200  //自己定义发送的数据大小 
#define SENDPORT 1348
#define RECEIVEPORT 8884

/********* GBN部分的全局变量   *******/
#define GETANDSEND 10 //收到10个顺序包后要发送ACK 


/***********/ 
WSADATA wsa;
int findA = 0;
int endFrameR = 0;//传送时设计一个结束帧，收到结束帧后就停止接收 
int endFrameS = 0;
char recvData[FRAMESIZE];//接收区 
char sendData[FRAMESIZE];//发送区 
struct frame getbuf;
struct frame sendbuf;

/********* GBN部分的全局变量   *******/
int getP = 0;//这里负责获得几个包发一次ACK 

int Read_file(FILE *fp,char *start){
	int i = 0;
	if(feof(fp)){
		return -1;
	}
	else{
		while((!feof(fp))&&i<DATASIZE){
			fscanf(fp,"%c",&start[i]);
			i++;
		}
	}
	return i;
}


int toSend(char *origin,int len,int n,char type){
	if (len == 0 || strlen(origin) == 0){
		return -1;
	}
	/*for(int i=0;i<n;i++){
		sendbuf.data[i] = origin[i];
	}*/
	strcpy(sendbuf.data,origin);
	sendbuf.length = len;
	sendbuf.checksum = getcrcc(origin,len);
	sendbuf.signal.seq = n;
	sendbuf.type = type;
	return sizeof(sendbuf);
	
}

int toData(char *getData,frame getbuf,int n){
	int flag = 0;
	if (getbuf.type == 'A'){ //Ack
		flag = 1; //接受到的为ACK
		n = getbuf.signal.ack;
	}else if (getbuf.type =='S'){
		flag = 2; //接受到的为seq
		n = getbuf.signal.seq;
	}
	//获取的数据有问题（zhao），并不能拆分出字符串 
	printf("length=%d\n",getbuf.length);
	if (getcrcc(getbuf.data,getbuf.length) == getbuf.checksum){
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
		//strcpy(getData,getbuf.data);
	}
	else{
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
		//strcpy(getData,getbuf.data);
		printf("---bad---\n");
		flag =-1; //传输过程遭到破坏
	}
	//
	return flag;
}
//==============================================================

DWORD WINAPI recv(LPVOID p){
	//printf("in recv thread.\n");
	sockaddr_in addr_in;
	SOCKET sclient;
	int len;
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	FILE *fp;
	int ret = 0;
	//printf("before slicent.\n");
	if (sclient == INVALID_SOCKET)
	{
		printf("socket failed\n");
		return -1;
	}
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(RECEIVEPORT);
	addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(sclient, (sockaddr*)&addr_in, sizeof(sockaddr)) == SOCKET_ERROR){
		printf("bind failed\n");
		return -1;
	}
	len = sizeof(addr_in);
	
	//建立连接 
	printf("waitforA.\n");
	ret = 0;
	while(findA == 0){
		memset(recvData, 0, sizeof(recvData));
		ret = recvfrom(sclient, recvData, sizeof(recvData), 0, (sockaddr *)&addr_in, &len);
		memcpy(&getbuf,recvData,sizeof(getbuf));
		if(getbuf.type == '1'){
			findA = 1;
		} 
	} 
	printf("findA!\n");
	
	while (!endFrameR){
		//Sleep(1000);
    	do{ 
			memset(recvData, 0, sizeof(recvData));
			//把接收到的信息转换成结构体,往数组里丢就可以,处理的事情线程管 (zhao)
			ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
			//TODO:这里不知道为什么有问题 
			clearF(getbuf);
			memcpy(&getbuf,recvData,sizeof(recvData));
			//TODO:WRONG??
			printf("getbuf.data=%s\n",getbuf.data);
			printf("getbuf.length=%d\n",getbuf.length);
			if(!full(GETQUE,0)){
				getin(GETQUE,getbuf,0);
			} 
    	}while(endFrameR == 0);
    	
    	if(endFrameR == 1){
    		printf("接收完成！\n");
    		break;
		}
	}
	closesocket(sclient);
	return 0;
}
 
DWORD WINAPI send(LPVOID p){
	//printf("in send thread.\n");
	int retval;
	SOCKET sclient;
	sockaddr_in addr_in;
	
	char data[DATASIZE]; 
	
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(SENDPORT);
	addr_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	//建立连接的帧	
	Frame firstC;
	firstC.type='1'; 
	memcpy(sendData, &firstC, sizeof(firstC));
			
	while(findA == 0){
		Sleep(1000); 
    	retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
	} 
	
	
	while (1){
		Sleep(2000);
        char filename[32]="sendtoA.txt";
        int i = 0,num = 0;
        FILE *fp=fopen(filename,"r");
        if(fp == NULL){
		    printf("打开错误！\n");
	    }
	    
        for(i = 0;fp!=NULL;i++){
        	memset(sendData,0,sizeof(sendData));
        	num = Read_file(fp,data);
        	if(num<=0){
        		endFrameS = 1;
        		memset(sendData,0,sizeof(sendData));
        		clearF(sendbuf);
        		sendbuf.type='E';
        		char endb[DATASIZE]={0};
        		strcpy(endb,"I am End.");
        		strcpy(sendbuf.data,endb);
        		sendbuf.data[DATASIZE-1]='P';
        		memcpy(sendData,&sendbuf,sizeof(sendbuf));
        		printf("End入队，共有%d\n",sendNum);
			}else{
				num = toSend(data,num,0,'S');
				//printf("sendbuf.length=%d\n",sendbuf.length);，发送无问题 
			}
			 
			if(sendBadReport == 0 && !full(SENDQUE,0)){
				memcpy(sendData,&sendbuf,sizeof(sendbuf));
				getin(SENDQUE,sendbuf,0);
			}else if (sendBadReport == 0 && full(SENDQUE,0)){
				printf("发送队列空!\n");
			}else{
				//---GBN---- 
				//TODO:发送报告错误的帧结构 （zhao&zhang）
				/* getin(SENDQUE,sendbuf,1) 代表把缓冲区sendbuf放入sendaque中
				 * 也就是发送ack的队列中
				 */ 
				sendBadReport = 0;
			}
			
			if(!empty(SENDQUE,1)){
				sendbuf = getout(SENDQUE,1);//优先发送ack 
			}else if(!empty(SENDQUE,0)){
				sendbuf = getout(SENDQUE,0);
			}else{
				printf("send队列空！\n");
			}
			
			retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
        	if (retval == SOCKET_ERROR){
				printf("sendt0 failed\n");
				closesocket(sclient);
				WSACleanup();
				return -1;
			}
			Sleep(1000);
			if(endFrameS == 1){
				break;
			}
		}
		fclose(fp);
		printf("num=%d\n",num); 
        if(num<=0){
    		printf("传输完成！\n");
    		break;
		}
	}
	closesocket(sclient);
	return 0; 
}
 
DWORD WINAPI deal(LPVOID p){
	//如果接收队列不为空，则把它拆出来（这里判断包序号，写GBN算法）（zhang）
	char rdata[DATASIZE];
	FILE *fp; 
	Frame getBuf;
	fp = fopen("recvFromA.txt","w");
	if(fp == NULL){
		printf("Create Error!\n");
		return -1;
	}
	while(!endFrameR){
		while(empty(GETQUE,0)) ;
		getBuf = getout(GETQUE,0);
		
		if(getBuf.type =='S'){ //是数据包 
			int n; //Attention：这里的n用来储存序号，不知道Go-Back-N算法用什么存，然后检查序号
			if (toData(rdata,getBuf,n) != -1){ //TODO:todata没办法正常判断 
				printf("S:成功拆分出数据\n");
			}
			else{
				printf("Seq:数据传输过程出现错误\n");
			}
			fprintf(fp,"%s",getbuf.data);
		}else if(getBuf.type = 'A'){
			//如果接收到ACK了，该做什么处理，GBN解决（zhang） 
			
		}else if (getBuf.type = 'E'){
			printf("I am End.\n");
			endFrameR = 1;
			break;
			//接收到结束帧 ,结束 
		}else if(getbuf.type == 'B'){
			clearQ(SENDQUE);
			sendBadReport = 1; 
		} 
			
	} 
	fclose(fp);
}

int main()
{
	error.type='-';
	HANDLE recvh, sendh ,dealh;
	int a = 0;
	DWORD recv_id, send_id, deal_id, recv_excode, send_excode, deal_excode;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("startup failed\n");
		return -1;
	}
	sendh = CreateThread(NULL,0,send,0,0,&send_id);//创建发送线程
	recvh = CreateThread(NULL,0,recv,0,0,&recv_id);
	dealh = CreateThread(NULL,0,deal,0,0,&deal_id);
	
	HANDLE m_hEvent[3];    
  //两事件  
  	m_hEvent[0]=sendh;  
  	m_hEvent[1]=recvh; 
	m_hEvent[2]=dealh; 
	WaitForMultipleObjects(3,m_hEvent,true,INFINITE);
	printf("\n*****end*****\n");
	getchar();
	system("pause");
	return 0;
}
 
 
  

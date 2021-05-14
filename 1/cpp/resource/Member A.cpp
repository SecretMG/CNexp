//客户端1
//#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "frame.h" 
#pragma comment(lib,"WS2_32.lib")

#define FRAMESIZE 3000 //自己定义,缓冲区大小（每次缓冲区内存放一个帧） 
#define DATASIZE 1200  //自己定义发送的数据大小 
#define SENDPORT 8884
#define RECEIVEPORT 1348
 
WSADATA wsa;
int findB = 0;
int endFrameS = 0;//传送时设计一个结束帧，收到结束帧后就停止接收
int endFrameR = 0; 
char recvData[FRAMESIZE];//接收区 
char sendData[FRAMESIZE];//发送区 
Frame test;

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

//===========================================================================================
//TODO:将读入信息(origin)封装成帧，放入buf中 ，len表示origin的长度,n表示帧序号 （zhao） 
//返回值为帧的长度，如果输入空数据，返回-1 
//TODO:发送的帧 

struct frame sendbuf;
int toSend(char *origin,int len,int n,char type){
	if (len == 0 || strlen(origin) == 0){
		return -1;
	}
	/*for(int i=0;i<n;i++){
		sendbuf.data[i] = origin[i];
		//printf("da")
	}*/
	strcpy(sendbuf.data,origin);
	sendbuf.length = len;
	sendbuf.checksum = getcrcc(origin,len);
	sendbuf.signal.seq = n;
	sendbuf.type = type;
	return sizeof(sendbuf);
	
}
struct frame getbuf;
struct frame end;

//接受到的东西放getData里，getbuf为收到的结构体，n为序号(zhao)
int toData(char *getData,frame getbuf,int n){
	int flag = 0;
	if (getbuf.type == 'A') //Ack
	{
		flag = 1; //接受到的为ACK
		n = getbuf.signal.ack;
	}
	else if (getbuf.type =='S'){
		flag = 2; //接受到的为seq
		n = getbuf.signal.seq;
	}
	if (getcrcc(getbuf.data,getbuf.length)==getbuf.checksum){
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
		
	}
	else{
		flag =-1; //传输过程遭到破坏
	}
	return flag;
}
//==============================================================

void clearF(Frame i){
	memset(i.data,0,sizeof(i.data));
	i.length = 0;
	i.type=0;
	return;
}

void endFrameBuild(){
	end.type='e';
	return;
}


DWORD WINAPI recv(LPVOID p){
	//printf("in recv thread.\n");
	 
	sockaddr_in addr_in;
	SOCKET sclient;
	int len;
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int ret = 0;
	
	if (sclient == INVALID_SOCKET){
		printf("socket failed\n");
		return -1;
	}
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(RECEIVEPORT);
	addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(sclient, (sockaddr*)&addr_in, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("bind failed\n");
		return -1;
	}
	len = sizeof(addr_in);
	//printf("socket success.\n");
	//建立连接 
	//Sleep(3000);
	
	while(findB == 0){
		//printf("waitforB.\n");
		memset(recvData, 0, sizeof(recvData));
		//recvData[ret] = 0x00;
		//TODO:发送建立连接的帧(zhao) 
		ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
		memcpy(&getbuf,recvData,sizeof(Frame));
		if(getbuf.type == '1'){
			findB = 1;
		}
		//Sleep(4);
	} 
	//printf("findB!\n");
	while (!endFrameR){
		Sleep(1000);
    	do{ 
			memset(recvData, 0, sizeof(recvData));
			//把接收到的信息转换成结构体,往数组里丢就可以,处理的事情线程管 (zhao)
			ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
			//printf("ret=%d,len=%d\n",ret,strlen(recvData));
			//TODO:这里不知道为什么有问题 
			clearF(getbuf);
			memcpy(&getbuf,recvData,sizeof(recvData));
			//printf("getbuf.data=%s\n",getbuf.data);
			if(!full(GETQUE)){
				getin(GETQUE,getbuf);
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
	printf("in send thread.\n");
	 
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
	strcpy(firstC.data,"I am test");
	memcpy(sendData, &firstC, sizeof(firstC));
			
	while(findB == 0){
		Sleep(1000); 
        memcpy(&test,sendData,sizeof(sendbuf));
        //printf("data:%s\n",test.data);
    	retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
	} 
	
	printf("findB.\n");
	while (1){
		Sleep(2000);
        char filename[32]="sendtoB.txt";
        int i = 0,num = 0;
        FILE *fp=fopen(filename,"r");
        if(fp == NULL){
		    printf("打开错误！\n");
	    }
	    
        for(i = 0;fp!=NULL;i++){
        	memset(sendData,0,sizeof(sendData));
        	num = Read_file(fp,data);//data中存放了帧的具体数据内容
        	/*memcpy(sendData,&sendbuf,sizeof(sendbuf));
        	clearF(test);
        	memcpy(&test,sendData,sizeof(sendbuf));*/
        	if(num<=0){
        		//TODO:发送end帧 (gui)
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
			}
			
			if(!full(SENDQUE)){
				memcpy(sendData,&sendbuf,sizeof(sendbuf));
				getin(SENDQUE,sendbuf);
			}else{
				printf("发送队列满!\n");
			}
			clearF(sendbuf);
			if(!empty(SENDQUE)){
				sendbuf = getout(SENDQUE);
			}else{
				printf("send队列空！\n");
			}
			printf("send start : '%c'\n",sendbuf.data[0]);
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
	fp = fopen("recvFromB.txt","w");
	if(fp == NULL){
		printf("Create Error!\n");
		return -1;
	}
	while(!endFrameR){
		while(empty(GETQUE)) ;
		getBuf = getout(GETQUE);
		
		//printf("getbuf.type=%c\n",getbuf.type);
		if(getBuf.type =='S'){ //是数据包 
			int n; //Attention：这里的n用来储存序号，不知道Go-Back-N算法用什么存，然后检查序号
			printf("data:%d,type=%c\n",strlen(getbuf.data),getbuf.type);
			if (toData(rdata,getbuf,n) != -1){ //todata没办法正常判断 
				printf("S:成功拆分出数据\n");
			}
			else{
				//printf("Seq:数据传输过程出现错误\n");
			}
			
			fprintf(fp,"%s",getbuf.data);
		}else if(getBuf.type = 'A'){
			//如果接收到ACK了，该做什么处理，GBN解决（zhang） 
		}else if (getBuf.type = 'E'){
			printf("getEnd!\n");
			endFrameR = 1;
			break;
			//接收到结束帧 ,结束 
		}else if(getBuf.type == 'B'){
			//接收到错误报告了，应该重新发送，需要清空自己的sendQue,并且放入某个该发的内容 
			//用它void clearQue(int type)
		} 
			
	} 
	fclose(fp);
}

int main()
{
	error.type='B';
	HANDLE recvh, sendh,dealh;
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
	Sleep(5000);
	return 0;
}
 
 
  

//客户端1
//#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "frame.h"
#pragma comment(lib,"WS2_32.lib")

#define BUFFERSIZE 255
 
WSADATA wsa;

int Read_file(FILE *fp,char *start){
	int i = 0;
	if(feof(fp)){
		return -1;
	}
	else{
		while((!feof(fp))&&i<255){
			fscanf(fp,"%c",&start[i]);
			i++;
		}
	}
	return i;
}

DWORD WINAPI recv(LPVOID p)
{
	char buf[20];
	sockaddr_in addr_in;
	SOCKET sclient;
	int len;
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sclient == INVALID_SOCKET)
	{
		printf("socket failed\n");
		return -1;
	}
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(6000);//接收6000端口传来的内容 
	addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(sclient, (sockaddr*)&addr_in, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		printf("bind failed\n");
		return -1;
	}
	len = sizeof(addr_in);
	FILE *fp;
	char recvData[BUFFERSIZE];
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		fp = fopen("recvFromA.txt","w");
		if(fp == NULL){
			printf("Create Error!\n");
			return -1;
		}
    	
    	int ret = 0;// = recvfrom(sclient, recvData, 255, 0, (sockaddr *)&sin, &len);
    	do
    	{
			memset(recvData, 0, sizeof(recvData));
			recvData[ret] = 0x00;
			ret = recvfrom(sclient, recvData, BUFFERSIZE, 0, (sockaddr *)&addr_in, &len);
        	printf("recv:%s\n",recvData);
        	fprintf(fp,"%s",recvData);
        	printf("ret = %d\n",ret);
    	}while(ret>0&&ret==255);
    	if(ret<255){
    		printf("接收完成！\n");
    		break;
		}
	}
	closesocket(sclient);
	return 0;
}
 
DWORD WINAPI send(LPVOID p)
{
	int retval;
	char buf[20];
	SOCKET sclient;
	sockaddr_in addr_in;
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(8000);//向9000端口发送 
	addr_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//***************
	while (1)
	{
		//printf("input the content:\n");
		memset(buf, 0, sizeof(buf));

        char sendData[BUFFERSIZE];
        char filename[32]="sendtoA.txt";
        int i = 0,num = 0;
        FILE *fp=fopen(filename,"r");
        if(fp == NULL){
		    printf("打开错误！\n");
	    }
	    
        for(i = 0;fp!=NULL;i++){
        	memset(sendData,0,sizeof(sendData));
        	num = Read_file(fp,sendData);//sendData中存放了帧的具体数据内容
        	//发送内容 
        	if(num<=0){
        		break;
			}
        	retval = sendto(sclient, sendData, BUFFERSIZE, 0, (sockaddr *)&addr_in, sizeof(addr_in));
        	if (retval == SOCKET_ERROR)
			{
				printf("sendt0 failed\n");
				closesocket(sclient);
				WSACleanup();
				return -1;
			}
        	//printf("num=%d,send:%s\n",num,sendData);
			Sleep(400);
		}
		fclose(fp);
        if(num<=0){
    		printf("传输完成！\n");
    		break;
		}
	}
	closesocket(sclient);
	return 0;
}
 
int main()
{
	HANDLE recvh, sendh;
	DWORD recv_id, send_id, recv_excode, send_excode;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("startup failed\n");
		return -1;
	}
	recvh = CreateThread(NULL, 0, recv, 0, 0, &recv_id);
	sendh = CreateThread(NULL, 0, send, 0, 0, &send_id);
	while (1)
	{
		Sleep(1000);
		GetExitCodeThread(recvh, &recv_excode);
		GetExitCodeThread(sendh, &send_excode);
		if (recv_excode != STILL_ACTIVE || send_excode != STILL_ACTIVE)
		{
			break;
		}
	}
	printf("\n*****end*****\n");
	getchar();
	system("pause");
	return 0;
}
 
 
  

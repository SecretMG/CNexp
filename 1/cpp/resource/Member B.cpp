//�ͻ���1
//#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "frame.h" 
#pragma comment(lib,"WS2_32.lib")

#define FRAMESIZE 3000 //�Լ�����,��������С��ÿ�λ������ڴ��һ��֡�� 
#define DATASIZE 1200  //�Լ����巢�͵����ݴ�С 
#define SENDPORT 1348
#define RECEIVEPORT 8884

/********* GBN���ֵ�ȫ�ֱ���   *******/
#define GETANDSEND 10 //�յ�10��˳�����Ҫ����ACK 


/***********/ 
 
WSADATA wsa;
int findA = 0;
int endFrameR = 0;//����ʱ���һ������֡���յ�����֡���ֹͣ���� 
int endFrameS = 0;
char recvData[FRAMESIZE];//������ 
char sendData[FRAMESIZE];//������ 
struct frame getbuf;
struct frame end;
struct frame sendbuf;

/********* GBN���ֵ�ȫ�ֱ���   *******/
int getP = 0;//���︺���ü�������һ��ACK 

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

void clearF(Frame i){
	memset(i.data,0,sizeof(i.data));
	i.length = 0;
	i.type=0;
	return;
}

int toData(char *getData,frame getbuf,int n){
	int flag = 0;
	if (getbuf.type == 'A'){ //Ack
		flag = 1; //���ܵ���ΪACK
		n = getbuf.signal.ack;
	}else if (getbuf.type =='S'){
		flag = 2; //���ܵ���Ϊseq
		n = getbuf.signal.seq;
	}
	//��ȡ�����������⣨zhao���������ܲ�ֳ��ַ��� 
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
		flag =-1; //��������⵽�ƻ�
	}
	//printf("")
	return flag;
}
//==============================================================

void endFrameBuild(){
	end.type='e';
	return;
}

void clearQue(int type){
	if(type == GETQUE){
		/*
		Frame getq[QUEUE_SIZE];//��Ż�õ�֡
		int getqStart = 0;//Ŀǰ���п�ʼ 
		int getqEnd = -1;
		int getNum = 0;*/
		memset(getq,0,sizeof(getq));
		getqStart = 0;
		getNum = 0;
		getqEnd = -1;
	}
	else if(type == SENDQUE){
		memset(sendq,0,sizeof(sendq));
		sendqStart = 0;
		sendNum = 0;
		sendqEnd = -1;
	}
	return;
} 

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
	
	
	//�������� 
	printf("waitforA.\n");
	ret = 0;
	while(findA == 0){
    	//һֱ�ȴ�A��������Ϣ 
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
			//�ѽ��յ�����Ϣת���ɽṹ��,�������ﶪ�Ϳ���,����������̹߳� (zhao)
			ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
			//printf("ret=%d,len=%d\n",ret,strlen(recvData));
			//TODO:���ﲻ֪��Ϊʲô������ 
			clearF(getbuf);
			memcpy(&getbuf,recvData,sizeof(recvData));
			//printf("getbuf.data=%s\n",getbuf.data);
			if(!full(GETQUE)){
				getin(GETQUE,getbuf);
			} 
    	}while(endFrameR == 0);
    	
    	if(endFrameR == 1){
    		printf("������ɣ�\n");
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
	
	//�������ӵ�֡	
	Frame firstC;
	firstC.type='1'; 
	memcpy(sendData, &firstC, sizeof(firstC));
			
	while(findA == 0){
		//���û�н��յ����ӣ���һֱ�ȴ�
		Sleep(1000); 
    	retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
	} 
	
	
	while (1){
		Sleep(2000);
        char filename[32]="sendtoA.txt";
        int i = 0,num = 0;
        FILE *fp=fopen(filename,"r");
        if(fp == NULL){
		    printf("�򿪴���\n");
	    }
	    
        for(i = 0;fp!=NULL;i++){
        	memset(sendData,0,sizeof(sendData));
        	num = Read_file(fp,data);//data�д����֡�ľ�����������
        	/*memcpy(sendData,&sendbuf,sizeof(sendbuf));
        	clearF(test);
        	memcpy(&test,sendData,sizeof(sendbuf));*/
        	if(num<=0){
        		//TODO:����end֡ (gui)
        		endFrameS = 1;
        		memset(sendData,0,sizeof(sendData));
        		clearF(sendbuf);
        		sendbuf.type='E';
        		char endb[DATASIZE]={0};
        		strcpy(endb,"I am End.");
        		strcpy(sendbuf.data,endb);
        		sendbuf.data[DATASIZE-1]='P';
        		memcpy(sendData,&sendbuf,sizeof(sendbuf));
        		printf("End��ӣ�����%d\n",sendNum);
			}else{
				num = toSend(data,num,0,'S');
			}
			
			if(!full(SENDQUE)){
				memcpy(sendData,&sendbuf,sizeof(sendbuf));
				getin(SENDQUE,sendbuf);
			}else{
				printf("���Ͷ��п�!\n");
			}
			if(!empty(SENDQUE)){
				sendbuf = getout(SENDQUE);
			}else{
				printf("send���пգ�\n");
			}
			sendbuf = getout(SENDQUE);
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
    		printf("������ɣ�\n");
    		break;
		}
	}
	closesocket(sclient);
	return 0; 
}
 
DWORD WINAPI deal(LPVOID p){
	//������ն��в�Ϊ�գ������������������жϰ���ţ�дGBN�㷨����zhang��
	char rdata[DATASIZE];
	FILE *fp; 
	Frame getBuf;
	fp = fopen("recvFromA.txt","w");
	if(fp == NULL){
		printf("Create Error!\n");
		return -1;
	}
	while(!endFrameR){
		while(empty(GETQUE)) ;
		getBuf = getout(GETQUE);
		
		//printf("getbuf.type=%c\n",getbuf.type);
		if(getBuf.type =='S'){ //�����ݰ� 
			int n; //Attention�������n����������ţ���֪��Go-Back-N�㷨��ʲô�棬Ȼ�������
			printf("data:%d,type=%c\n",strlen(getbuf.data),getbuf.type);
			if (toData(rdata,getbuf,n) != -1){ //todataû�취�����ж� 
				printf("S:�ɹ���ֳ�����\n");
			}
			else{
				//printf("Seq:���ݴ�����̳��ִ���\n");
			}
			fprintf(fp,"%s",getbuf.data);
		}else if(getBuf.type = 'A'){
			//������յ�ACK�ˣ�����ʲô����GBN�����zhang�� 
		}else if (getBuf.type = 'E'){
			printf("I am End.\n");
			endFrameR = 1;
			break;
			//���յ�����֡ ,���� 
		}else if(getBuf.type == 'B'){
			//���յ����󱨸��ˣ�Ӧ�����·��ͣ���Ҫ����Լ���sendQue,���ҷ���ĳ���÷������� 
			//����void clearQue(int type)
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
	sendh = CreateThread(NULL,0,send,0,0,&send_id);//���������߳�
	recvh = CreateThread(NULL,0,recv,0,0,&recv_id);
	dealh = CreateThread(NULL,0,deal,0,0,&deal_id);
	//TODO:�ٿ�һ�����̴���֡��gui��  
	//dealh = CreateThread(NULL, 0, deal, 0, 0, &deal_id);
	/*
	while (1)
	{
		Sleep(3000);
		GetExitCodeThread(recvh, &recv_excode);//��ȡ�߳�״̬ 
		GetExitCodeThread(sendh, &send_excode);//��ȡ�߳�״̬ 
		//GetExitCodeThread(dealh, &deal_excode);
		//printf("recv=%d,send=%d\n",recv_excode,send_excode);
		if (recv_excode != STILL_ACTIVE && send_excode != STILL_ACTIVE){
			break;
		}
	}*/
	HANDLE m_hEvent[3];    
  //���¼�  
  	m_hEvent[0]=sendh;  
  	m_hEvent[1]=recvh; 
	m_hEvent[2]=dealh; 
	WaitForMultipleObjects(3,m_hEvent,true,INFINITE);
	printf("\n*****end*****\n");
	getchar();
	system("pause");
	return 0;
}
 
 
  

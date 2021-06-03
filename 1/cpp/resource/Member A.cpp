//�ͻ���1
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

#define FRAMESIZE 3000 //�Լ�����,��������С��ÿ�λ������ڴ��һ��֡�� 
#define DATASIZE 1200  //�Լ����巢�͵����ݴ�С 
#define RECEIVEPORT 1348
 
WSADATA wsa;
int findB = 0;
int endFrameS = 0;//����ʱ���һ������֡���յ�����֡���ֹͣ����
int endFrameR = 0; 
char recvData[FRAMESIZE];//������ 
char sendData[FRAMESIZE];//������ 
struct frame getbuf;
int recvnum = 0;//�ڼ��ν��� 
int sendnum = 0;//�ڼ��ν��� 

Frame test;
struct frame sendbuf;

void Init(const char *a){
	readConfig(a);
	nextseq = InitSeqNo;//��һ�����͵����
	window = (Frame *)malloc(sizeof(Frame)*SWSize);
	
}

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
		//printf("da")
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
		flag = 1; //���ܵ���ΪACK
		n = getbuf.signal.ack;
	}
	else if (getbuf.type =='S'){
		flag = 2; //���ܵ���Ϊseq
		n = getbuf.signal.seq;
	}
	if (getcrcc(getbuf.data,getbuf.length)==getbuf.checksum){
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
		
	}
	else{
		printf("bad!\n");
		flag =-1; //��������⵽�ƻ�
	}
	return flag;
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
	
	while(findB == 0){
		memset(recvData, 0, sizeof(recvData));
		ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
		memcpy(&getbuf,recvData,sizeof(Frame));
		if(getbuf.type == '1'){
			findB = 1;
		}
	} 
	
	while (!endFrameR){
		Sleep(1000);
    	do{ 
			memset(recvData, 0, sizeof(recvData));
			ret = recvfrom(sclient, recvData, FRAMESIZE, 0, (sockaddr *)&addr_in, &len);
			clearF(getbuf);
			memcpy(&getbuf,recvData,sizeof(recvData));
			if(!full(GETQUE,0)){
				getin(GETQUE,getbuf,0);
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
	printf("in send thread.\n");
	 
	int retval;
	SOCKET sclient;
	sockaddr_in addr_in;
	
	char data[DATASIZE]; 
	
	sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(UDPPort);
	addr_in.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	//�������ӵ�֡	
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
		    printf("�򿪴���\n");
	    }
	    
        for(i = 0;fp!=NULL;i++){
        	memset(sendData,0,sizeof(sendData));
        	if(winFull == 0){
        		num = Read_file(fp,data);//data�д����֡�ľ�����������
			} 
        	if(num<=0){
        		//����end֡
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
				if(winFull == 0){ 
					num = toSend(data,num,nextseq,'S');
					//ErrorRate���� 
					if(isError() == 1){
						 char e;
						 int pos;
						 srand(time(NULL));
						 e = rand()%26 + 'a';
						 pos = rand()%300;
						 if(pos<sizeof(data)){
						 	sendbuf.data[pos] = e;
						 }
					}
				}
				
			}
			
			//���ACK������Ҫ���������ѭ��֮���ٴ��� 
			if(getACK == 1){
				if(nextseq < base + SWSize){ 
        			nextseq++;
        			winFull = 0;
    			}
    			else{
    				winFull = 1;
				}
    			getACK = 0;
			}
    		
			if(sendBadReport == 0 && !full(SENDQUE,0)){
				memcpy(sendData,&sendbuf,sizeof(sendbuf));
				getin(SENDQUE,sendbuf,0);
			}else if (sendBadReport == 0 && full(SENDQUE,0)){
				printf("���Ͷ��п�!\n");
			}else{
				//TODO:���ͱ�������֡�ṹ 
			}
			
			clearF(sendbuf);
			if(!empty(SENDQUE,1)){
				sendbuf = getout(SENDQUE,1);//���ȷ���ack 
				retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
		        	if (retval == SOCKET_ERROR){
						printf("sendt0 failed\n");
						closesocket(sclient);
						WSACleanup();
						return -1;
					}
			}else if(!empty(SENDQUE,0)){
				//TODO:����������ˣ���û�յ�ack����Ӧ�ü������� 
				if(isLost() == 0 && winFull == 0){
					sendbuf = getout(SENDQUE,0);
					//��ô��ʱ�Ǹ����� 
					retval = sendto(sclient, sendData, sizeof(sendData), 0, (sockaddr *)&addr_in, sizeof(addr_in));
		        	if (retval == SOCKET_ERROR){
						printf("sendt0 failed\n");
						closesocket(sclient);
						WSACleanup();
						return -1;
					}
				}
			}else{
				printf("send���пգ�\n");
			}
			
			Sleep(1000);
			if(endFrameS == 1){
				break;
			}
		}
		fclose(fp);
		//printf("num=%d\n",num); 
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
	FILE *fpout; 
	Frame getBuf;
	fp = fopen("recvFromB.txt","w");
	fp = fopen("recvFromB_.txt","w");
	if(fp == NULL){
		printf("Create Error!\n");
		return -1;
	}
	while(!endFrameR){
		while(empty(GETQUE,0)) ;
		getBuf = getout(GETQUE,0);
		
		//printf("getbuf.type=%c\n",getbuf.type);
		if(getBuf.type =='S'){ //�����ݰ� 
			int n; //Attention�������n����������ţ���֪��Go-Back-N�㷨��ʲô�棬Ȼ�������
			printf("data:%d,type=%c,num=%d\n",strlen(getbuf.data),getbuf.type,getbuf.length);
			if (toData(rdata,getbuf,n) != -1){ //todataû�취�����ж� 
				printf("S:�ɹ���ֳ�����\n");
				if(getbuf.signal.seq!=n){
					printf("%d,pdu_exp=%d,pdu_recv=%d,status=OK\n",recvnum,n,getbuf.signal.seq);
					fprintf(fp,"%s",getbuf.data);
				}else{
					printf("%d,pdu_exp=%d,pdu_recv=%d,status=NoErr\n",recvnum,n,getbuf.signal.seq);
				}
				
			}
			else{
				printf("Seq:���ݴ�����̳��ִ���\n");
				printf("%d,pdu_exp=%d,pdu_recv=%d,status=DataErr\n",recvnum,n);
				//TODO:��Ҫ��ʱ�ش����������ش��� 
			} 
		}else if(getBuf.type = 'A'){
			//TODO:������յ�ACK�ˣ�����ʲô����GBN�����zhang�� 
			//���յ�ACK֮����Ҫ������ţ��������ڣ�������Ҫ�ݴ�һЩFrame 
			if(getBuf.signal.ack >= base){
			//ack����Ŵ��ڵ��ڴ�ȷ��֡����� 
			waitcount = 0;
			if(nextseq = totalseq+1){
				flag = 1;//ȫ��֡������� 
				printf("���ݴ���ȫ����ɣ�����\n");
			}
			else{
				base = getBuf.signal.ack + 1;//��һ����ȷ��֡�����Ϊack����ż�һ 
			} 
		}
		}else if (getBuf.type = 'E'){
			printf("getEnd!\n");
			endFrameR = 1;
			break;
			//���յ�����֡ ,���� 
		}else if(getBuf.type == 'B'){
			//���յ����󱨸��ˣ�Ӧ�����·��ͣ���Ҫ����Լ���sendQue,���ҷ���ĳ���÷������� 
			clearQ(SENDQUE);
			sendBadReport = 1;//�Ӵ��ڵ�ͷ��ʼ���ͣ������п�ͷ��  
		} 
			
	} 
	fclose(fp);
}

int main()
{
	Init("AConfig.txt");
	error.type='B';
	HANDLE recvh, sendh,dealh;
	int a = 0;
	DWORD recv_id, send_id, deal_id, recv_excode, send_excode, deal_excode;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("startup failed\n");
		return -1;
	}
	sendh = CreateThread(NULL,0,send,0,0,&send_id);//���������߳�
	recvh = CreateThread(NULL,0,recv,0,0,&recv_id);
	dealh = CreateThread(NULL,0,deal,0,0,&deal_id);
	
	HANDLE m_hEvent[3];    
  	//���¼�  
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
 
 
  

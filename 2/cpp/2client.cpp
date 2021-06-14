#include <iostream>
#include <pthread.h>
#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#pragma comment(lib, "ws2_32")

#include "network2.h"

#define STOPNUM 3 //ctrl+c
#define STOPNUM2 75 //K
#define PAUSENUM 16 //ctrl+p
#define PAUSENUM2 80 //p
#define CONTINUE 19 //ctrl+s 
#define MAXNUM 10
#define UPDATENUM 1

int socketfd;
int slen,clen;
struct sockaddr_in sAddr;
struct sockaddr_in cAddr;
char initFile[20] = {0};
int tableMutex[100]={0};
int haveTimeOut = 0;
int change[10]={0};
int cnum=0; 
int changeGet = -1;
int nowNew = 0;

//table.find(sData[i].DestNode))->second.dist
void *send(void* args){
	DVAlg *d = (DVAlg*) args; 
	char sendArray[5000]={0};
	long int nowTime = 0;
	sendData sData;
	char rbuff[5000]={0};
	char sbuff[5000]={0};
	int num = 0;
	int time = 0;
	int j = 0;
	//�����ܶ��socket 
	
	while(1){
		//Sleep(10000);
		while(!d->running) ;
		Sleep(1000);
		//printf("%d\n",d->neiNum);
		//printf("size=%d\n",d->table.size());
		for(int j = 0;j<d->neiNum;j++){
		for(auto i = d->table.begin();i!=d->table.end();i++ ){ //����Ҫ���͵ı���
			
				Sleep(300);
				sData.SrcNode = d->UDPPort; //���ھӷ����Լ���port�� 
				sData.DestNode = i->first; 
				sData.SendtoWho = d->neighbor[j];//��Ҫ��������ھӹ�����ı��� 
				sData.Distance = (i->second).dist;
				//printf("send %d->%d,dis = %f\n",d->UDPPort,i->first,(i->second).dist) ;
				while(i->second.tableMutex == 1);
				i->second.tableMutex = 1;
				if(haveTimeOut == 0 && d->isNeighbor(i->second.port) == true){ 
					/*if((i->second.lastRecvTime)!=-1 && time >= ((d->config).MaxValidTime)*40){
						change[cnum++]=i->second.port;
						sData.Distance = -1;
						haveTimeOut = 1;
						break;
					}else*/if((i->second.lastRecvTime) ==-1 &&clock() - d->createTime >= ((d->config).MaxValidTime)*40){
						//printf("havne get %d\n",i->second.port);
						change[cnum++]=i->second.port;
						sData.Distance = -1;
						haveTimeOut = 1;
						break;
					}
				}
				i->second.tableMutex = 0;
				//printf("s1\n");
				
				if(sData.Distance!=-1){
					//sData.Distance = i.second.dist; //����
					memcpy(sbuff,&sData,sizeof(sData));
					num = sendto(socketfd, sbuff, sizeof(sbuff), 0, (sockaddr *)&(sAddr), sizeof(sAddr));
					//printf("send dis=%f\n",sData.Distance);
					if (num == SOCKET_ERROR){
						printf("%d\n",GetLastError());
						Sleep(10000);
					}
				}
			//send
			}
			
			if(cnum!=0){
				for(int k = 0;k<cnum;k++){
					d->TimeOut(change[k]);
				}
				cnum = 0;
			}
			changeGet = 0;
			
		}
		
		Sleep(500);
	}
	closesocket(socketfd);
	WSACleanup();
	return 0;
}

void *recv(void* args){
	DVAlg *d = (DVAlg*) args;
	int i = 0;
	sendData rData[10];
	char rbuff[5000];
	int j =0;
	int num = 0;
	char string[20]={"log"};
	char a[10];
	itoa(d->UDPPort,a,5);
	strcat(string,a);
	strcat(string,".txt\0");
	while(1){
		while(!d->running) ;
		for(j = 0;j<UPDATENUM;){
			Sleep(200);
			slen = sizeof(sAddr);
			num = recvfrom(socketfd, rbuff, sizeof(rbuff), 0, (sockaddr *)&(sAddr), &slen);/*���շ���˵����ݣ�recv���������û�����ݻ�����*/
			if(num == -1 ){
				continue;
			}
			memcpy(&rData[j],rbuff,sizeof(rData[j]));
			if(d->UDPPort!= rData[j].SendtoWho){
				continue;
			}
			//�������ʱ��
			//printf("des:%d\n",rData[j].DestNode);
			if(d->table.find(rData[j].SrcNode) != d->table.end()){
				//err src����dest�� 
				while((d->table.find(rData[j].SrcNode)->second).tableMutex == 1);
				(d->table.find(rData[j].SrcNode)->second).tableMutex = 1;
				(d->table.find(rData[j].SrcNode)->second).lastRecvTime = clock();
				(d->table.find(rData[j].SrcNode)->second).tableMutex = 0;
			}
			while(changeGet == 1);
			//printf("sendto=%d,get:%d %d %f\n",rData[j].SendtoWho,rData[j].DestNode,rData[j].SrcNode,rData[j].Distance);
			d->updateTable(UPDATENUM,rData,d->newget,string);
			changeGet = 1;
			j++;
			
		}	
		
		Sleep(1000);
	}
}

void *getBoard(void *args){
	DVAlg *d = (DVAlg*) args;
	char ch;
	while(1){
		//TODO:�����������벢����״̬ת��
		ch = getch();
		ch = toupper(ch);
		switch (ch){
			case STOPNUM:case STOPNUM2:{
				printf("Stop.\n");
				d->init = 0;
				d->running = 0;
				break;
			}
			case PAUSENUM:case PAUSENUM2:{
				printf("Pause.\n");
				d->pause = 1;
				d->running = 0;
				break;
			}
			case CONTINUE:{
				printf("Continue.\n");
				d->pause = 0;
				d->running = 1;
				break;
			}
			default:{
				closesocket(socketfd);
				exit(0);
				break;
			}
		} 
	}
}

int main(int argc, char* argv[]){
	strcpy(initFile,argv[3]); 
	
	DVAlg dv(argv[1],atoi(argv[2]),argv[3]);//��ʼ����Ҫ�����ھӱ� 
	dv.ReadConfig("1.txt",dv.config); 
	//dv.createTime = clock();
	dv.getTable(argv[3]);printf("1\n");
	pthread_t tids[5];
	WSADATA wsadata;
	WORD w_req = MAKEWORD(2, 2);//�汾��
	int err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "��ʼ���׽��ֿ�ʧ�ܣ��˳��߳�" << endl;
		return 0;
	}else{
		cout << "��ʼ���׽��ֿ�ɹ���" << endl;
	}
	
	socketfd = socket(AF_INET, SOCK_DGRAM, 0);
  	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sAddr.sin_port = htons(51000);//�����������ˣ���������˿�
	slen = sizeof(sAddr);
	
	cAddr.sin_family = AF_INET;              
	cAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //����ip
	cAddr.sin_port = htons(dv.UDPPort);//�����˿�
	clen = sizeof(cAddr);
	bind(socketfd, (struct sockaddr *) & cAddr, clen);
	dv.init = 1;
	while(1){
		if(dv.init == 0){
			dv.init = 1;
			dv.ReadConfig("1.txt",dv.config); 
			dv.getTable(argv[3]);
			dv.UDPPort = atoi(argv[2]);
			dv.createTime = clock();
		}
		int ret = pthread_create(&tids[1], NULL, recv, (void *)(&dv));
		if (ret != 0){
		    cout << "pthread_create error: error_code=" << ret << endl;
		}
		//printf("recvs\n"); 
		ret = pthread_create(&tids[0], NULL, send, (void *)(&dv));
		if (ret != 0){
		    cout << "pthread_create error: error_code=" << ret << endl;
		}
		//printf("send,s\n"); 
		ret = pthread_create(&tids[2], NULL, getBoard, (void *)(&dv));
		if (ret != 0){
		    cout << "pthread_create error: error_code=" << ret << endl;
		}//timeout
		
		pthread_join(tids[0],NULL);
		pthread_join(tids[1],NULL);
		pthread_join(tids[2],NULL);
	}	
	
}

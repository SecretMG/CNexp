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

//server 
int listenfd;
int len;
sockaddr_in cAddr,sAddr;
int cAddress[100] ={0};
int cNum = 0;
sockaddr_in client[100];
//recv queue
int recvHead = 0;
int recvTail = -1;
int recvNum = 0;
 
sendData rec[100];

bool findSocket(int port){
	int i = 0;
	for(i = 0;i<cNum;i++){
		if(cAddress[i]== port){
			return true;
		}
	}
	return false;
}

//需要开收发线程，收到的放队列里，send转发出去 
void *recv(void* args){
	DVAlg *d = (DVAlg*) args;
	int i = 0;
	sendData rData;
	char rbuff[5000];
	int len;
	int num = 0;
	
	while(d->running){
		Sleep(500);
		len = sizeof(sAddr);
		num = recvfrom(listenfd, rbuff, sizeof(rbuff), 0, (sockaddr *)&(cAddr), &len);
		if(num == -1){
			cout<<"recv error!"<<endl;
			exit(0);
		}
		memcpy(&rData,rbuff,sizeof(rData));
		rec[++recvTail]=rData;
		recvNum++;
		//printf("recv:%d,get:%d %d %f\n",num,rData.DestNode,rData.SrcNode,rData.Distance);
		if(findSocket(cAddr.sin_port)==false){
			cout<<"get a new member!\n"<<endl;
			//TODO:复制 
			client[cNum].sin_family = cAddr.sin_family;
			client[cNum].sin_addr.S_un.S_addr = cAddr.sin_addr.S_un.S_addr; 
			client[cNum].sin_port = cAddr.sin_port;
			cAddress[cNum] = cAddr.sin_port;
			cNum++;
		}
		
		Sleep(100);
	}
}

void *send(void* args){
	DVAlg *d = (DVAlg*) args;
	char sbuff[5000]={0};
	sendData rData;
	int num = 0; 
	
	while(1){
		//TODO：先采用广播形式,A超时仍收到A的内容 
		while(!d->running);
		if(recvNum == 0){
			continue;
		}
		rData = rec[recvHead];
		recvHead++;
		recvNum--;
		memcpy(sbuff,&rData,sizeof(rData));
		for(int i = 0;i<cNum;i++){
			Sleep(100);
			if(client[i].sin_port != cAddr.sin_port){
				len = sizeof(client[i]);
				num = sendto(listenfd, sbuff, sizeof(sbuff), 0, (sockaddr *)&(client[i]), len);
				printf("To:%d send:%d %d %f\n",rData.SendtoWho,rData.DestNode,rData.SrcNode,rData.Distance);
				if(num == -1){
					cout<<"发送失败"<<endl;
				} 
			}
		}
	}
	Sleep(1000);
	closesocket(listenfd);
	WSACleanup();
	return 0;
}
 
int main(int argc, char* argv[]){
	DVAlg dv;
	char ch;
	sendData sData;//发送
	int num = 0; 
	char rbuff[5000]={0};
	pthread_t tids[5];
	WSADATA wsadata;
	int err;
	WORD w_req;
	
	dv.UDPPort = 51000;
	w_req = MAKEWORD(2, 2);//版本号
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！退出线程" << endl;
		return 0;
	}else{
		cout << "初始化套接字库成功！" << endl;
	}
	
	
	//否则创建套接字
  	listenfd = socket(AF_INET, SOCK_DGRAM, 0);
  	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sAddr.sin_port = htons(51000);
	if( bind(listenfd, (struct sockaddr*)&sAddr, sizeof(sAddr)) == -1){
		printf("%d\n",GetLastError());
		exit(0);
	}	
	
	while(1){
		while(dv.running == 0) ;//未运行则阻塞
		//没有初始化，则需要初始化
		if(dv.stop == 1 || dv.pause == 1){
			continue;
		}
		len = sizeof(cAddr);
		while(dv.running == 1){
			//接收到信息，先判断客户端是否存在，未存在则加入表
			int ret = pthread_create(&tids[1], NULL, recv, (void *)&dv);
			if (ret != 0){
			    cout << "pthread_create error: error_code=" << ret << endl;
			}
			
			ret = pthread_create(&tids[0], NULL, send, (void *)&dv);
			if (ret != 0){
			    cout << "pthread_create error: error_code=" << ret << endl;
			}
			
			pthread_join(tids[0],NULL);
			pthread_join(tids[1],NULL);
			Sleep(1000);		
		}	 
	}
	closesocket(listenfd);
	return 0; 
}  

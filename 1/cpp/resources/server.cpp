#include <winsock2.h>
#include <windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#include "frame.h" 
#include "settings.h"
#include "GBN.h"

#pragma comment(lib,"WS2_32.lib")

 
#define MAXLINE 6000
//#define FRAMESIZE 4500
#define GETSENDNUM 1
//TODO:无法接收到后边的 
/*-----socket使用变量-------*/ 
int fd;
int listenfd;
int connfd;
DWORD read_id, write_id;
HANDLE recvh, sendh;
int addrlen;
sockaddr_in server_addr;  /*服务器地址结构*/
sockaddr_in client_addr;  /*客户端地址结构*/
int retval;
int len;
WSADATA wsa;

/*-------GBN使用变量---------- */ 
//1.超时发送
int cStart = 0;//是否需要开始计时,用CLOCK计时 
clock_t startTime = 0;
clock_t endTime = 0;
int reSend = 0;
//2.滑动窗口
int startF = 0;//本次开始帧,知道窗口大小，就知道结尾帧是什么了
int ACKCheckNum = 0;//确认到了哪个ACK 
int ACKSendNum = -1;//应该发送哪个序号的ACK 
//3.结束线程相关
int endFrameR = 0;//传送时设计一个结束帧，收到结束帧后就停止接收 
int endFrameS = 0;
int endACK = 0;
//4.发送接收数组等
struct frame sendbuf;
struct frame getbuf; 
//5.结束标志
int endFrameSeq; 
int endT = 0;//当endT = 3时，全部线程结束 


/*-----读取数据相关-----*/
FILE *now_fp;//记录上次读取位置 
int a_lock = 0;
int a = 0;

/*-------debug工具-----------*/
void printSendbuf(frame out){
	printf("---sendbuf info---\n");
	printf("seq=%d,ack=%d\n",out.signal.seq,out.signal.ack);
	printf("type=%c,length=%d\n",out.type,out.length);
	printf("data=%s\n",out.data);
	printf("---end info---\n");
}  

void printGetbuf(frame get){
	printf("---getBuf info---\n");
	printf("seq=%d,ack=%d\n",get.signal.seq,get.signal.ack);
	printf("type=%c,length=%d\n",get.type,get.length);
	printf("data=%s\n",get.data);
	printf("---end info---\n");
} 

/*-----初始化工具------*/
void Init(const char *a){
	readConfig(a);
	//1.清空队列 
	clearQ(0);
	clearQ(1);
	//2.初始化某些变量 
	winFull = 0;
	reSend = 0;
	return;
}
 
 
/*----数据处理工具----*/
//这里需要记录指针位置？ 要修改 
int Read_file(FILE *fp,char *start){
	int i = 0;
	if(feof(fp)){
		return -1;
	}
	else{
		while( (!feof(fp))&&i< DataSize ){
			fscanf(fp,"%c",&start[i]);
			i++;
		}
	}
	//nowfp = fp;
	return i;
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
	
	if (getcrcc(getbuf.data,getbuf.length) == getbuf.checksum){
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
	}
	else{
		for(int i=0;i<getbuf.length;i++){
			getData[i] = getbuf.data[i];
		}
		//printf("---bad---\n");
		flag = -1; //传输过程遭到破坏
	}
	return flag;
}


/*---帧形成工具---*/
int toACK(int n){
	memset(sendbuf.data,0,sizeof(sendbuf.data));
	sendbuf.length = 0;
	sendbuf.signal.ack = n;
	sendbuf.type = 'A';
	sendbuf.checksum = 0;
	sendbuf.signal.seq = 0;
	return sizeof(sendbuf);
} 

int toSendFrame(char *origin,int len,int n,char type){
	if (len == 0 || strlen(origin) == 0){
		return -1;
	}
	strcpy(sendbuf.data,origin);
	sendbuf.length = len;
	sendbuf.checksum = getcrcc(origin,len);
	sendbuf.signal.seq = n;
	sendbuf.type = type;
	return sizeof(sendbuf);
}

void toEndFrame(){
	clearF(sendbuf);
	sendbuf.type='E';
	char endb[20]={0};
	memset(sendbuf.data,0,sizeof(sendbuf.data));
	strcpy(endb,"I am End.");
	strcpy(sendbuf.data,endb);
	sendbuf.data[DataSize-1]='P';
}

void toFirstFrame(const char *filename){
	memset(first.data,0,sizeof(first.data));
	FILE *flen =fopen(filename,"r");
	int length = 0;
	fseek(flen,0,SEEK_END);
	length = (int)(ftell(flen));
	first.length = (int)(length/DataSize);
	if(length%DataSize!=0){
		first.length++;
	}
	first.length--;
	first.type = '1';
	fclose(flen);
	return;
}

/*----其他工具----*/
//滑动窗口
void winMove(int &cStart,int &getACK){
		while(a_lock==1) ;
	a_lock = 1;
	if(getACK == 1){
		int j = 0;
		
		for(;j<a;j++){
			getout(SENDQUE,0);
			startF=(startF+1)%QUEUE_SIZE;
			if(winFull>0){
				winFull--;
			} 
		}
		
	}
	a_lock = 0;
	getACK = 0;
	//cStart = 1;//重置计时器 
	return;
}

/*-----读取线程-----*/
DWORD WINAPI readFun(LPVOID p){
	char readbuff[MAXLINE];
	int num = 0;
	len = sizeof(client_addr);
	
	while(endT!=3){
		num = recvfrom(connfd, readbuff, MAXLINE, 0, (sockaddr *)&client_addr, &len);/*接收服务端的数据，recv在这里如果没有数据会阻塞*/
		memcpy(&getbuf,readbuff,sizeof(getbuf));
		//printGetbuf(getbuf);
		if(!full(GETQUE,0)){
			getin(GETQUE,getbuf,0);
			//printGetbuf();
		} 
		//endFrameR = 1; 
	}
	//printf("接收完成!\n");
	return 0;
}

/*-----发送线程-----*/
DWORD WINAPI writeFun(LPVOID p){
	char sendbuff[MAXLINE];
	char write[4000];
	unsigned char i = 0;
	int num = 0;
	int n = 1;
	
	char filename[32]="sendtoB.txt";
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		printf("打开文件失败!\n");
		return 0;
	}
	getACK = 0;
	//先发送结束ACK号，1024结束为0 
	
	memset(&sendbuff,0,sizeof(sendbuff));
	toFirstFrame(filename);
	memcpy(sendbuff,&first,sizeof(sendbuf));
	while(n--){
		printf("send First.\n"); 
		retval = sendto(connfd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&client_addr, sizeof(client_addr));
		Sleep(1000);
	}
	
	memset(&sendbuff,0,sizeof(sendbuff));
	
	 
	while(endT!=3){
		Sleep(200);
		if (winFull < SWSize){
			//此时可以发送帧
			num = Read_file(fp,write);
			if(num>0){
				memset(sendbuf.data,0,sizeof(sendbuf.data));
				num = toSendFrame(write,num,nextseq,'S'); 
				if(num < DataSize){
					sendbuf.data[num]='\0';
				}
				nextseq++; 
			}
		
		} 
		
		
		while(a_lock == 1);
		a_lock = 1;
		//发送队列处理
		if(!full(SENDQUE,0)){
			if(winFull == 0){
				startF = (QUEUE_SIZE + sendqStart)% QUEUE_SIZE;
				cStart = 1;
			} 
			//winFull = 1代表放入1个 
			if(winFull < SWSize){
				getin(SENDQUE,sendbuf,0);
				//memcpy(sendData,&sendbuf,sizeof(sendbuf));
				winFull++;
			} 
		}else if(full(SENDQUE,0)){
			printf("发送队列满!\n");
		}else{
			//发送错误报告 
		}
		//printf("ACKSendNum = %d\n",ACKSendNum);
		if(ACKSendNum != -1){
			toACK(ACKSendNum);
			getin(SENDQUE,sendbuf,1);
			ACKSendNum = -1; 
		} 
		
		//优先ACK> 普通帧
		
		if(!empty(SENDQUE,1)){
			sendbuf = getout(SENDQUE,1);
			memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
			//printSendbuf(sendbuf);
			retval = sendto(connfd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&client_addr, sizeof(client_addr));
        	if (retval == SOCKET_ERROR){
				printf("send sendt0 failed\n");
				closesocket(fd);
				WSACleanup();
				return -1;
			}
			if(sendbuf.signal.ack == endFrameSeq){
				endT = endT | 2;
			}
		}  
		if(!empty(SENDQUE,0)){ 
			//printf("reSend =%d\n",reSend); 
			if(winFull+1 < SWSize && reSend == 0){
				//为什么-2,需要考虑 
				sendbuf = outWho((startF+winFull-1)%QUEUE_SIZE);
				//printf("send who:%d\n",sendbuf.signal.seq);
				if(isError() == 1){
					 srand(time(NULL));
					 char e =  rand()%26 + 'a';
					 int pos = rand()%300;
					 if( pos < DataSize){
					 	sendbuf.data[pos] = e;
					 }
				}
				memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
				//printSendbuf(sendbuf);
				if(isLost()==0){
					retval = sendto(connfd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&client_addr, sizeof(client_addr));
		        	if (retval == SOCKET_ERROR){
						printf("send2 sendt0 failed\n");
						closesocket(fd);
						WSACleanup();
						return -1;
					}
				}
				
			}else if(reSend == 1){
				//1.计时器归0
				cStart = 1;
				//2.重传 
				//printf("startF=%d,startNo=%d\n",startF,sendq[startF].signal.seq);
				for(int j=0;j<SWSize && j< winFull ;j++){
					cStart = 1; 
					Sleep(200);
					sendbuf = outWho((startF+j)%QUEUE_SIZE);
					memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
					//printSendbuf(sendbuf); 
					if(isLost()==0){
					retval = sendto(connfd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&client_addr, sizeof(client_addr));
		        	if (retval == SOCKET_ERROR){
						printf("send2 sendt0 failed\n");
						closesocket(fd);
						WSACleanup();
						return -1;
					}
				}
				}
				//3.设置resend = 0
				reSend = 0; 
			}
		}else{
			//printf("send队列空！\n");
		}
		Sleep(200);
		a_lock = 0;
	}
	fclose(fp); 
	//printf("发送完成!\n");
	return 0;
}

/*-----处理线程-----*/ 
DWORD WINAPI deal(LPVOID p){
	int n = 0;
	char rdata[4000];
	FILE *fp,*fplog;
	Frame getBuf;
	int recvnum;
	int sumnum = 0; 
	
	fp = fopen("recvFromB.txt","w");
	fplog = fopen("Alog.txt","w");
	if(fp == NULL || fplog == NULL){
		printf("Create Error!\n");
		return -1;
	}
	while(endT !=3){
		while(empty(GETQUE,0)) ;
		getBuf = getout(GETQUE,0);
		//printGetbuf(getBuf);
		
		if(getBuf.type =='S'){
			if(n>endFrameSeq){
				continue;
			}
			if (toData(rdata,getBuf,n) != -1){ 
				recvnum = getBuf.length;
				if(getBuf.signal.seq == n){ 
					if(n > endFrameSeq){ //接收到了结束帧，且无错误 
						endT = endT | 1;
						continue;
					} 
					printf("%d,pdu_exp=%d,pdu_recv=%d,status=OK\n",recvnum,n,getBuf.signal.seq);
					fprintf(fplog,"%d,pdu_exp=%d,pdu_recv=%d,status=OK\n",recvnum,n,getBuf.signal.seq);
					for(int k =0;k<getBuf.length;k++){
						fprintf(fp,"%c",getBuf.data[k]);
					}
					if(++sumnum >= GETSENDNUM){
						ACKSendNum = n;
						sumnum = 0;
					}
					//printf("get data,and ACKSendNum=%d\n",ACKSendNum);
					n++;
					
				}else{
					if(getBuf.signal.seq < n){
						ACKSendNum = getBuf.signal.seq;
					}
					printf("%d,pdu_exp=%d,pdu_recv=%d,status=NoErr\n",recvnum,n,getBuf.signal.seq);
					fprintf(fplog,"%d,pdu_exp=%d,pdu_recv=%d,status=NoErr\n",recvnum,n,getBuf.signal.seq);	
				}		
			}else{
				recvnum = getBuf.length;
				printf("%d,pdu_exp=%d,pdu_recv=%d,status=DataErr\n",recvnum,n,getBuf.signal.seq); 
				fprintf(fplog,"%d,pdu_exp=%d,pdu_recv=%d,status=DataErr\n",recvnum,n,getBuf.signal.seq);
			}
			if(n > endFrameSeq){
				//printf("getEnd!\n");
				ACKSendNum = getBuf.signal.seq; 
			}
			while(a_lock == 1) ;
			a_lock = 1;
			ACKCheckNum = -1;
			a_lock = 0; 
		}else if(getBuf.type == 'A'){
			//1.拆包，获得ACK的序号
			//printf("base=%d,getbuf.signal.ack=%d\n",base,getBuf.signal.ack); 
			if(getBuf.signal.ack >= base){
				while(a_lock == 1) ;//必须读过一次才能继续往后滑动 
				a_lock = 1;
				ACKCheckNum = getBuf.signal.ack - base + 1;//往后滑动几个
				for(int j = 0;j<ACKCheckNum;j++){
					getout(SENDQUE,0);
					startF=(startF+1)%QUEUE_SIZE;
					if(winFull>0){
						winFull--;
					} 
				}
				a_lock = 0;
				if(base < endFrameSeq){
					base = getBuf.signal.ack + 1;
				}
				cStart = 1;
			}
			if(base > endFrameSeq){
				endT = endT | 2;
			}
			getACK = 1;
		}else if(getBuf.type == 'B'){
			clearQ(GETQUE);
			sendBadReport = 1;  
		}else if(getBuf.type == '1'){
			endFrameSeq = getBuf.length;
		} 
		//printf("endT=%d\n",endT);
		if(endT == 3){
			endFrameR = 1;
			endFrameS = 1;
			endACK = 1;
			break;
		}
			
	} 
	fclose(fp);
	fclose(fplog);
}
 
/*-----计时器线程-----*/
DWORD WINAPI count(LPVOID p){
	while(endT!=3){
		if(cStart == 1){
			startTime = clock();
			cStart = 0;
		}
		//printf("now time:%d\n",clock()-startTime);
		if(clock()-startTime > Timeout*1000){
			//printf("timeout!\n");
			startTime = 0;
			reSend = 1;
		}
	} 
}

int main(int argc, char** argv){
	Init("Aconfig.txt"); 
	char buff[MAXLINE];
	int num;
	HANDLE recvh, sendh,counth,dealh;
	HANDLE mt[4]; 
	DWORD recv_id, send_id,count_id,deal_id;
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("startup failed\n");
		return -1;
	}
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1){
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}
	
	/*设置服务端地址*/
	addrlen = sizeof(struct sockaddr_in);
	len = sizeof(struct sockaddr_in);
	memset(&server_addr, 0, addrlen);
	server_addr.sin_family = AF_INET;    /*AF_INET表示 IPv4 Intern 协议*/
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*INADDR_ANY 可以监听任意IP */
	server_addr.sin_port = htons(UDPPort); /*设置端口*/
    
	/*绑定地址结构到套接字描述符*/
	if( bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}
 
	/*设置监听队列，这里设置为1，表示只能同时处理一个客户端的连接*/
	if( listen(listenfd, 1) == -1){
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}
	
	//signal(SIGINT,stop); /*注册SIGINT 信号*/
	while(1){	
		printf("wait client accept \n");
		if( (connfd = accept(listenfd, (struct sockaddr*)&client_addr, &addrlen)) == -1){
			printf("accept socket error: %s(errno: %d)",strerror(errno),errno);/*接收客户端的连接，这里会阻塞，直到有客户端连接*/
			//exit(0);
		}
		recvh = CreateThread(NULL,0,readFun,0,0,&recv_id);
		if(recvh == NULL){
			printf("pthread_create write_func err\n");
		}
		
		counth = CreateThread(NULL,0,count,0,0,&count_id);
		if(counth == NULL){
			printf("pthread_count err\n");
		}
		
		dealh = CreateThread(NULL,0,deal,0,0,&deal_id);
		if(dealh == NULL){
			printf("pthread_deal err\n");
		}
		
		sendh = CreateThread(NULL,0,writeFun,0,0,&send_id);
		if(sendh == NULL){
			printf("pthread_create read_func err\n");
		}
		mt[0] = sendh;
		mt[1] = recvh;
		mt[2] = counth;
		mt[3] = dealh; 
		WaitForMultipleObjects(4,mt,true,INFINITE);
		//printf("write pthread out \n");
	}
	closesocket(listenfd);
 	WSACleanup();
}
 
 

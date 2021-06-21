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
#define GETSENDNUM 1 //ÿ5����һ��ACK 

/*-----socketʹ�ñ���-------*/ 
int fd;
int connfd;
DWORD read_id, write_id;
HANDLE recvh, sendh;
int addrlen;
sockaddr_in server_addr;  /*��������ַ�ṹ*/
sockaddr_in client_addr;  /*�ͻ��˵�ַ�ṹ*/
int retval;
int len;
WSADATA wsa;


/*-------GBNʹ�ñ���---------- */ 
//1.��ʱ����
int cStart = 0;//�Ƿ���Ҫ��ʼ��ʱ,��CLOCK��ʱ 
clock_t startTime = 0;
clock_t endTime = 0;
int reSend = 0;
//2.��������
int startF = 0;//���ο�ʼ֡,֪�����ڴ�С����֪����β֡��ʲô��
int ACKCheckNum = -1;//ȷ�ϵ����ĸ�ACK 
int ACKSendNum = -1;//Ӧ�÷����ĸ���ŵ�ACK 
//3.�����߳����
int endFrameR = 0;//����ʱ���һ������֡���յ�����֡���ֹͣ���� 
int endFrameS = 0;
int endACK = 0;
//4.���ͽ��������
struct frame sendbuf;
struct frame getbuf; 
//5.������־
int endFrameSeq; 
int endT;//��endT = 3ʱ��ȫ���߳̽��� 


/*-----��ȡ�������-----*/
FILE *now_fp;//��¼�ϴζ�ȡλ�� 

/*---��ʱ����---*/
int a_lock = 0;
int a = 0;
int a_read = 0;
/*-------debug����-----------*/
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

/*-----��ʼ������------*/
void Init(const char *a){
	readConfig(a);
	//1.��ն��� 
	clearQ(0);
	clearQ(1);
	//2.��ʼ��ĳЩ���� 
	winFull = 0;
	reSend = 0;
	ACKSendNum = -1;
	return;
}
 
 
/*----���ݴ�����----*/
//������Ҫ��¼ָ��λ�ã� Ҫ�޸� 
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
		flag = 1; //���ܵ���ΪACK
		n = getbuf.signal.ack;
	}else if (getbuf.type =='S'){
		flag = 2; //���ܵ���Ϊseq
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
		flag = -1; //��������⵽�ƻ�
	}
	return flag;
}


/*---֡�γɹ���---*/
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

/*----��������----*/
//��������
void winMove(int &cStart,int &getACK){
	while(a_lock==1) ;
	a_lock = 1;
	if(getACK == 1){
		int j = 0;
		
	}
	a_read = 1;
	a_lock = 0;
	getACK = 0;
	//cStart = 1;//���ü�ʱ�� 
	return;
}

/*-----��ȡ�߳�-----*/
DWORD WINAPI readFun(LPVOID p){
	char readbuff[MAXLINE];
	int num = 0;
	
	while(endT !=3){
		num = recvfrom(fd, readbuff, MAXLINE, 0, (sockaddr *)&server_addr, &len);/*���շ���˵����ݣ�recv���������û�����ݻ�����*/
		memcpy(&getbuf,readbuff,sizeof(getbuf));
		/*if(getbuf.signal.ack !=0){
			printGetbuf(getbuf);
		}*/
		
		if(!full(GETQUE,0)){
			getin(GETQUE,getbuf,0);
			//printGetbuf();
		} 
		//endFrameR = 1; 
	}
	//printf("�������!\n");
	return 0;
}

/*-----�����߳�-----*/
DWORD WINAPI writeFun(LPVOID p){
	char sendbuff[MAXLINE];
	char write[4000];
	unsigned char i = 0;
	int num = 0;
	
	char filename[32]="sendtoA.txt";
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		printf("���ļ�ʧ��!\n");
		return 0;
	}
	getACK = 0;
	//�ȷ��ͽ���ACK�ţ�1024����Ϊ0 
	memset(&sendbuff,0,sizeof(sendbuff));
	toFirstFrame(filename);
	memcpy(sendbuff,&first,sizeof(sendbuf));
	retval = sendto(fd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&server_addr, sizeof(server_addr));
	memset(&sendbuff,0,sizeof(sendbuff));
	 
	while(endT !=3){
		Sleep(200);
		
		if (winFull < SWSize){
			//��ʱ���Է���֡
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
		
		
		//void winMove(int &cStart,int &getACK)
		while(a_lock == 1);
		winMove(cStart,getACK);
		//���Ͷ��д���
		a_lock = 1;
		if(!full(SENDQUE,0)){
			if(winFull == 0){
				startF = (QUEUE_SIZE + sendqStart)% QUEUE_SIZE;
			} 
			//winFull = 1�������1�� 
			if(winFull < SWSize){
				getin(SENDQUE,sendbuf,0);
				winFull++;
			} 
		}else if(full(SENDQUE,0)){
			printf("���Ͷ�����!\n");
		}else{
			//���ʹ��󱨸� 
		}
		if(ACKSendNum != -1){
			toACK(ACKSendNum);
			getin(SENDQUE,sendbuf,1);
			ACKSendNum = -1; 
		} 
		//����ACK> ��ͨ֡
		if(!empty(SENDQUE,1)){
			sendbuf = getout(SENDQUE,1);
			memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
			retval = sendto(fd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&server_addr, sizeof(server_addr));
        	if (retval == SOCKET_ERROR){
				printf("sendt0 failed\n");
				closesocket(fd);
				WSACleanup();
				return -1;
			}
			if(sendbuf.signal.ack == endFrameSeq){
				endT = 1;
			}
		} 
		if(!empty(SENDQUE,0)){ 
			if(winFull+1 < SWSize && reSend == 0){
				//Ϊʲô-2,��Ҫ���� 
				sendbuf = outWho((startF+winFull-1)%QUEUE_SIZE);
				if(isError() == 1){
					 srand(time(NULL));
					 char e =  rand()%26 + 'a';
					 int pos = rand()%300;
					 if( pos < DataSize){
					 	sendbuf.data[pos] = e;
					 }
				}
				memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
				if(isLost()==0){
					retval = sendto(fd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&server_addr, sizeof(server_addr));
		        	if (retval == SOCKET_ERROR){
						printf("send2 sendt0 failed\n");
						closesocket(fd);
						WSACleanup();
						return -1;
					}
				} 
			}else if(reSend == 1){
				//1.��ʱ����0
				cStart = 1;
				//2.�ش� 
				for(int j=0;j< SWSize && j < winFull;j++){ 
					sendbuf = outWho((startF+j)%QUEUE_SIZE);
					memcpy(sendbuff,&sendbuf,sizeof(sendbuf));
					//printSendbuf(); 
					Sleep(200);
					if(isLost()==0){
						retval = sendto(fd, sendbuff, sizeof(sendbuff), 0, (sockaddr *)&server_addr, sizeof(server_addr));
			        	if (retval == SOCKET_ERROR){
							printf("send2 sendt0 failed\n");
							closesocket(fd);
							WSACleanup();
							return -1;
						}
					} 
				}
				//3.����resend = 0
				reSend = 0; 
			}
		}else{
			//printf("send���пգ�\n");
		}
		a_lock = 0;
		Sleep(200);
	}
	fclose(fp); 
	//printf("�������!\n");
	return 0;
}

/*-----�����߳�-----*/ 
DWORD WINAPI deal(LPVOID p){
	int n = 0;
	char rdata[MAXLINE];
	FILE *fp,*fplog;
	Frame getBuf;
	int recvnum;
	int sumnum = 0; 
	
	fp = fopen("recvFromA.txt","w");
	fplog = fopen("Blog.txt","w");
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
					if(n > endFrameSeq){ //���յ��˽���֡�����޴��� 
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
			ACKCheckNum = -1; 
		}else if(getBuf.type == 'A'){
			//1.��������ACK�����
			//printf("base=%d,getbuf.signal.ack=%d\n",base,getBuf.signal.ack); 
			if(getBuf.signal.ack >= base){
				//1.1 ack�����>=��ȷ��֡�����
				while(a_lock == 1) ;//�������һ�β��ܼ������󻬶� 
				a_lock = 1;
				ACKCheckNum = getBuf.signal.ack - base + 1;//���󻬶�����
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
 
/*-----��ʱ���߳�-----*/
DWORD WINAPI count(LPVOID p){
	while(endT!=3){
		if(cStart == 1){
			startTime = clock();
			cStart = 0;
		}
		if(clock()-startTime > Timeout*1000){
			startTime = 0;
			reSend = 1;
		}
	} 
}
 
/*----client����----*/
void client_process(void){
	HANDLE recvh, sendh,dealh,counth;
	//������߳�������->����->���� 
	DWORD recv_id, send_id,deal_id,count_id;
	recvh = CreateThread(NULL,0,readFun,0,0,&recv_id);
	sendh = CreateThread(NULL,0,writeFun,0,0,&send_id);
	dealh = CreateThread(NULL,0,deal,0,0,&deal_id);
	counth = CreateThread(NULL,0,count,0,0,&count_id);
	while(endT!=3) ;
	
	printf("write pthread out \n");
}


int main(int argc, char** argv){
	Init("Bconfig.txt"); 
	char buff[MAXLINE];
	int cr;
	int num;
	
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("startup failed\n");
		return -1;
	}
	
 	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd == -1){
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}
	
	/*���÷���˵�ַ*/
	addrlen = sizeof(struct sockaddr_in);
	memset(&server_addr, 0, addrlen);
	server_addr.sin_family = AF_INET;    /*AF_INET��ʾ IPv4 Intern Э��*/
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*INADDR_ANY ���Լ�������IP */
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(UDPPort); /*���ö˿�*/
	len = sizeof(struct sockaddr_in);
	cr = connect(fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(cr == -1){
		printf("connect socket error: %s(errno: %d)\n",strerror(errno),errno);
		//exit(0);
	}
	
	/*�����ĸ��߳�*/
	client_process();
	closesocket(fd);
	return 0; 
}
 
 

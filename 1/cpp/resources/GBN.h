#ifndef GBN_H
#define GBN_H
#include "frame.h"

int winsizesend = 10;//���ͷ��������ڴ�С 
int base = 0;//�ѷ��ʹ�ȷ�ϵ�����֡ 
int nextseq;//��һ�������͵�֡ 
int totalseq;//��֡�� 
int winsizerecv = 1;//���մ��ڴ�С 
int waitseq;//�ڴ��յ���֡ 
int recvseq;//�յ���֡ 
int waitcount = 0;//������ 
int flag = 0;//�ж��Ƿ������ 

int getACK = 0;//�Ƿ���յ�ack�����յ�������+����
int sendBadReport = 0; 
int winFull; 

int sendACK;//����ACK 


#endif 

#ifndef GBN_H
#define GBN_H
#include "frame.h"

int winsizesend = 10;//发送方滑动窗口大小 
int base = 0;//已发送待确认的最早帧 
int nextseq;//下一个待发送的帧 
int totalseq;//总帧数 
int winsizerecv = 1;//接收窗口大小 
int waitseq;//期待收到的帧 
int recvseq;//收到的帧 
int waitcount = 0;//计数器 
int flag = 0;//判断是否发送完成 

int getACK = 0;//是否接收到ack，接收到则负责处理+发送
int sendBadReport = 0; 
int winFull; 

int sendACK;//发送ACK 


#endif 

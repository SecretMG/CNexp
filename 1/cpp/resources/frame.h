#ifndef FRAME_H
#define FRAME_H

#define Length 4096
#define N 9
#define QUEUE_SIZE 10
#define SENDQUE 0 //这里是type 
#define GETQUE 1
#define EMPTY 1
#define NOTEMPTY 0
#define FULL 1
#define NOTFULL 0

#define ER -1
#define SUCCESS 1 

#include <stdio.h>
#include <string.h>
static unsigned short crc16_ccitt_table[] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

/*获取crcc校验码*/
short getcrcc(char *buff,int length){
    short crc = 0xFFFF;
    for(int i=0;i<length;i++){
        crc = crc16_ccitt_table[(crc >> 8 ^ *buff++) & 0xff] ^ (crc << 8);
    }
    return crc;

}
/*数据帧结构体定义*/

typedef struct{
	int ack; //确认帧的序号
    int seq;//发送帧的序号
}Signal;
typedef struct frame
{
    char type; //说明是哪种数据结构的帧
    Signal signal;
    char data[Length];//数据字段，实验要求不超过4kB，此处设为4KB
    int length;//数据字段的长度
    short checksum;  //2个字节的crcc校验码
}Frame;

Frame getq[QUEUE_SIZE];//获得队列 
int getqStart = 0;
int getqEnd = -1;
int getNum = 0;



Frame senda[QUEUE_SIZE];//发送ack队列 
int sendaStart = 0; 
int sendaEnd = -1;
int sendaNum = 0;

Frame sendq[QUEUE_SIZE];//发送普通帧队列 
int sendqStart = 0;
int sendqEnd = -1;
int sendqNum = 0;

Frame error;//错误帧格式，目前没有用到 
Frame first;

int empty(int type,int who){
	//get需要判空 
	if(type == GETQUE && getNum == 0){
		return EMPTY;
	}else if(type == SENDQUE && sendqNum == 0 && who == 0){
		return EMPTY;
	}else if(type == SENDQUE && sendaNum == 0 && who == 1){
		return EMPTY;
	}else{
		return NOTEMPTY;
	} 	
} 

int full(int type,int who){
	//send队列需要判满 
	 if(type == GETQUE && getNum == QUEUE_SIZE){
	 	return FULL;
	 }else if (who == 0 &&type == SENDQUE && sendqNum == QUEUE_SIZE ){
	 	return FULL;
	 }else if (who == 1 &&type == SENDQUE && sendqNum == QUEUE_SIZE){
	 	return FULL;
	 }else{
	 	return NOTFULL;
	 }
}

Frame getout(int type,int who){
	//获得一个帧,参数为GETQUE 
	if(type == GETQUE){
		if(!empty(GETQUE,0)){
			getqEnd=(getqEnd+1)%QUEUE_SIZE;
			getNum--;
			Frame &r = getq[getqEnd];
			//printf("r.seq=%d,r.ack=%d\n",r.signal.seq,r.signal.ack);
			return r; 
		}else{
			return error;
		}
	}else if(type == SENDQUE){
		if(who == 0){ 
			if(!empty(SENDQUE,who)){
				sendqEnd=(sendqEnd+1)%QUEUE_SIZE;
				sendqNum--;
				return sendq[sendqEnd]; 
			}else{
				return error;
			}
		}else{
			if(!empty(SENDQUE,who)){
				sendaEnd=(sendaEnd+1)%QUEUE_SIZE;
				sendaNum--;
				return senda[sendaEnd]; 
			}else{
				return error;
			}
		}
		
	}else{
		return error;
	}
	
	
}

Frame outWho(int num){
	// 0 1 2 
	/*printf("-----outwho:%d----\n",num);
	printf("sendnum:%d\n",sendNum);
	printf("data:%s\n",sendq[num].data);*/
	return sendq[num];
}

int getin(int type,Frame in,int who){ //TODO:还需要往某个队列做一个备份？ 
	//只有send队列需要 
	if(type == GETQUE){
		if(!full(GETQUE,0)){
			getq[getqStart].type = in.type;
			getq[getqStart].length = in.length;
			strcpy(getq[getqStart].data,in.data);
			getq[getqStart].checksum = in.checksum;
			getq[getqStart].signal.ack = in.signal.ack;
			getq[getqStart].signal.seq = in.signal.seq;
			getqStart = (getqStart+1)%QUEUE_SIZE;
			getNum++;
			return SUCCESS;
		}else{
			return ER;
		}
	}else if(type == SENDQUE){
		if(who == 0){
			if(!full(SENDQUE,who)){
				sendq[sendqStart] = in;
				sendqStart = (sendqStart+1)%QUEUE_SIZE;
				sendqNum++;
				return SUCCESS;
			}else{
				return ER;
			}
		}else if(who == 1){
			if(!full(SENDQUE,who)){
				senda[sendaStart] = in;
				sendaStart = (sendaStart+1)%QUEUE_SIZE;
				sendaNum++;
				return SUCCESS;
			}else{
				return ER;
			}
		}
		
	}else{
		return ER;
	}
	
} 

void clearF(Frame i){
	memset(i.data,0,sizeof(i.data));
	i.length = 0;
	i.type=0;
	return;
}

void clearQ(int type){
	if(type == SENDQUE){
		sendqNum = 0;
		sendqStart = 0;
		sendqEnd = -1;
	}else{
		getqStart = 0;//目前队列开始 
	    getqEnd = -1;
		getNum = 0;
	}
	return;
}

int getSendNum(int who){
	if(who == 0){
		return sendqNum;
	}
	return sendqNum;
}
#endif

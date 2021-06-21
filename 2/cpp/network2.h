#ifndef NETWORK2
#define NETWORK2
#include <iostream>
#include <time.h>
#include <string.h>
#include <unordered_map> //hash表
#include <cmath>

#include <mutex>
using namespace std;

//路由表 结构体
typedef struct table
{
	 int port; //邻居节点
	 float dist; //最短距离
	 long int lastSendTime;//上一次发送时间 
	 long int lastRecvTime;
	 int tableMutex;
}Table;

//配置读进去的结构体
typedef struct configuration{
	int Frequency; //定期时间间隔，单位为毫秒
	float Unreachable;//不可达代价或者距离
	int MaxValidTime;//最大等待时间，单位为毫秒

}Config;

//传送时所用的结构体
typedef struct sendData
{
	int SendtoWho;
	int SrcNode;	//发送节点 id 
    int DestNode;	//目的结点 id 
    float Distance;	//到达目的节点的距离或代价。可以不是整数
    int seq;
}sendData;
 //char: 目标节点
//Hash表用于保存本节点的路由信息,char为目标节点
int infoNum = 0;


//发的时候是一条一条发

//更新的时候是 全部接受了 再更新
class DVAlg{
public:
	int init;
	int stop;
	int running;
	int pause; //接收键盘按键的内容
	int friends; 
	int UDPPort;
	int createTime=0;
	char name[20];
	int neighbor[20];//存放50001这种UDPport 
	int neiNum;
	int newget = 0;
	Config config;//时间
	unordered_map<int,Table> table;//存放信息的哈希表 
	std::mutex my_mutex;
	
	DVAlg();
	DVAlg(char *Name,int Port,char *configName); 
	void ReadConfig(char *path,Config &config);
	void getTable(char *path);
	void updateTable(int total,sendData *sendData,int seq,char *logName);
	void setTime(int des,long int time); 
	bool isSend(int des,long int time);
	bool isNeighbor(int port); 
	void TimeOut(int port);
private:
	
};

DVAlg::DVAlg(){
	stop = 0;
	running = 1;
	pause = 0;
	init = 1;
	neiNum = 0;
	newget = 0;
	createTime = clock();
}

DVAlg::DVAlg(char* Name,int Port,char *configName){
	strcpy(name,Name);
	UDPPort = Port;
	neiNum = 0;
	getTable(configName);
	stop = 0;
	running = 1;
	pause = 0;
	init = 1;
	newget = 0;
	createTime = clock();
	
}

void DVAlg::ReadConfig(char *path,Config &config){
	FILE *fp;            /*文件指针*/
	 if((fp = fopen(path,"r")) == NULL){
		 perror("fail to read1");
		 exit (1) ;
	}
	// fscanf(fp,"%s = %d\n",temp,&UDPPort);
	char *temp;
	fscanf(fp,"%s = %d\n",&temp,&config.Frequency); //定期时间间隔
	fscanf(fp,"%s = %f\n",&temp,&config.Unreachable); //不可达距离
	fscanf(fp,"%s = %d\n",&temp,&config.MaxValidTime); //最大等待时间
	printf("%d %f %d\n",config.Frequency,config.Unreachable,config.MaxValidTime);
	return;
}

void DVAlg::getTable(char *path){
	FILE *fp;
	//printf("Path2=%s\n",path);
	if((fp = fopen(path,"r")) == NULL){
		 perror("fail to read2");
		 exit (1) ;
	}
	char id;
	int index;
	int udp;
	float cost;
	Table temp;
	while (!feof(fp)){
		fscanf(fp,"%c %f %d\n",&id,&cost,&udp);
		temp.port=udp;
		temp.dist =cost;
		temp.lastSendTime = -1;
		temp.lastRecvTime = -1;
		neighbor[neiNum++] = udp;
		temp.tableMutex = 0;
		table.insert({udp,temp});
		//printf("get From Table:id=%c,cost=%f,udp=%d\n",id,cost,udp); 
	}
	return;
}

void DVAlg::updateTable(int total,sendData *sData,int seq,char *logName){
	int source = sData[0].SrcNode;//由于从同一个节点收的，所以source都一样

	/*
	1.不在表中：取出接受信息放入表中
	2.在表中，与原距离比较
		-小于原距离，则将原距离更新
		-其他，则不操作
	*/
	//printf("in Updata.\n");
	if(seq  < newget){
		return;
	}else{
		newget = seq;
	}
	for(int i=0;i<total;i++){ 
		if(table.find(sData[i].DestNode) == table.end()){ //此时该节点不在表项中
			Table temp;
			auto restry = table.find(sData[i].SrcNode);
			//printf("%f\n",restry->second.dist);
			temp.dist = (restry->second).dist + sData[i].Distance;  //计算距离
			//printf("1.2\n");
			temp.port = sData[i].SrcNode;
			//printf("1.3\n");
			//printf("%d %d",sData[i].DestNode,sData[i].SrcNode);
			table.insert({sData[i].DestNode,temp}); //把该节点插入hash表
			//printf("%d",table.find(sData[i].DestNode)->second.port);
			//printf("insert hash.\n");
		}
		else if(sData[i].DestNode != UDPPort &&sData[i].SrcNode!=sData[i].DestNode){
			int dis = (table.find(sData[i].SrcNode)->second).dist + sData[i].Distance;
			if(dis < (table.find(sData[i].DestNode))->second.dist){ //更新后的表项距离小于原表项
				//printf("2.3.1\n");
				Table temp;
				temp.dist = dis;
				//printf("2.3.2\n");
				temp.port = sData[i].SrcNode;
				temp.lastRecvTime = clock();
				table[sData[i].DestNode] = temp;
				//destnode存的是1-3距离，现在src是1-2距离，要结合发送来的2-3距离 
			}else if(dis > (table.find(sData[i].DestNode))->second.dist){
				printf("now get %d->%d,%f,must update %d->%d",sData[i].SrcNode,sData[i].DestNode,sData[i].Distance,UDPPort,sData[i].DestNode);
				auto restry = table.find(sData[i].DestNode); //找到1-3 距离，如果是从邻居发过来的 
				if((restry->second).port == sData[i].SrcNode){  //如果此时表项种的邻居节点也是一样的话 
					//修改的原因是路过的点一致，但路径上的距离发生变化（如有节点出问题等），也需要更新 
					(table.find(sData[i].DestNode))->second.dist = dis; //你要更改des的问题 
					newget ++;
				}
			}
		}
	}
	//打印信息 到显示台
	printf("\n");
	printf("##Sent. Source Node = %d ; Senquence Number = %d\n",source,seq);
	int index = 0;
	for(auto i:table){
		printf("# %d DestNode =  %d ;  Distance = %f; Neighbor = %d\n",index,i.first,i.second.dist,i.second.port);
		index += 1;
		//printf("lastRecvTime = %d\n",i.second.lastRecvTime);
	}
	printf("\n");

	//写入信息 到log.txt
	 FILE *fpWrite=fopen(logName,"w");
	 fprintf(fpWrite,"\n##Sent. Source Node = %d ; Senquence Number = %d\n",source,seq);
	 index = 0;
	 for(auto i:table){
		fprintf(fpWrite,"# %d DestNode =  %d ;  Distance = %f; Neighbor = %d\n",index,i.first,i.second.dist,i.second.port);
		index += 1;
	}
	fprintf(fpWrite,"\n");
	fclose(fpWrite);

}

void DVAlg::setTime(int des,long int time){
	//这里需要设置时间
	if(table.find(des) != table.end()){
		//这个节点在表中，重置时间 
		(table.find(des)->second).lastSendTime = time;
		//printf("settime:time=%d\n",time);
	}
	return;
	
}

bool DVAlg::isSend(int des,long int time){
	//判断是否到时间需要发送 
	if((table.find(des)->second).lastSendTime == -1){
		return true;
	}
	if( table.find(des) != table.end()){
		//这个节点在表中
		
		int time = clock()-(table.find(des)->second).lastSendTime;
		//TODO:zhel 
		//printf("in table.time=%d,max=%d\n",time,config.MaxValidTime);
		if(time >= config.Frequency*10){
			//printf("time end.\n");
			return true;
		}
	}
	return false;
	
}

bool DVAlg::isNeighbor(int port){
	int i = 0;
	for(;i<neiNum;i++){
		if(port == neighbor[i]){
			return true;
		}
	}
	return false;
}

void DVAlg::TimeOut(int port){
	for(auto it=table.begin();it!=table.end(); ){
		if((it->second).port == port ){
			printf("erase %d\n",(it->second).port); 
			it = table.erase( it );		// 方法一：执行it++指向下一个节点
		}
		else{
			it++;
		}
	}
	return;
}

#endif

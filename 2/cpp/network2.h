#ifndef NETWORK2
#define NETWORK2
#include <iostream>
#include <time.h>
#include <string.h>
#include <unordered_map> //hash��
#include <cmath>

#include <mutex>
using namespace std;

//·�ɱ� �ṹ��
typedef struct table
{
	 int port; //�ھӽڵ�
	 float dist; //��̾���
	 long int lastSendTime;//��һ�η���ʱ�� 
	 long int lastRecvTime;
	 int tableMutex;
}Table;

//���ö���ȥ�Ľṹ��
typedef struct configuration{
	int Frequency; //����ʱ��������λΪ����
	float Unreachable;//���ɴ���ۻ��߾���
	int MaxValidTime;//���ȴ�ʱ�䣬��λΪ����

}Config;

//����ʱ���õĽṹ��
typedef struct sendData
{
	int SendtoWho;
	int SrcNode;	//���ͽڵ� id 
    int DestNode;	//Ŀ�Ľ�� id 
    float Distance;	//����Ŀ�Ľڵ�ľ������ۡ����Բ�������
    int seq;
}sendData;
 //char: Ŀ��ڵ�
//Hash�����ڱ��汾�ڵ��·����Ϣ,charΪĿ��ڵ�
int infoNum = 0;


//����ʱ����һ��һ����

//���µ�ʱ���� ȫ�������� �ٸ���
class DVAlg{
public:
	int init;
	int stop;
	int running;
	int pause; //���ռ��̰���������
	int friends; 
	int UDPPort;
	int createTime=0;
	char name[20];
	int neighbor[20];//���50001����UDPport 
	int neiNum;
	int newget = 0;
	Config config;//ʱ��
	unordered_map<int,Table> table;//�����Ϣ�Ĺ�ϣ�� 
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
	FILE *fp;            /*�ļ�ָ��*/
	 if((fp = fopen(path,"r")) == NULL){
		 perror("fail to read1");
		 exit (1) ;
	}
	// fscanf(fp,"%s = %d\n",temp,&UDPPort);
	char *temp;
	fscanf(fp,"%s = %d\n",&temp,&config.Frequency); //����ʱ����
	fscanf(fp,"%s = %f\n",&temp,&config.Unreachable); //���ɴ����
	fscanf(fp,"%s = %d\n",&temp,&config.MaxValidTime); //���ȴ�ʱ��
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
	int source = sData[0].SrcNode;//���ڴ�ͬһ���ڵ��յģ�����source��һ��

	/*
	1.���ڱ��У�ȡ��������Ϣ�������
	2.�ڱ��У���ԭ����Ƚ�
		-С��ԭ���룬��ԭ�������
		-�������򲻲���
	*/
	//printf("in Updata.\n");
	if(seq  < newget){
		return;
	}else{
		newget = seq;
	}
	for(int i=0;i<total;i++){ 
		if(table.find(sData[i].DestNode) == table.end()){ //��ʱ�ýڵ㲻�ڱ�����
			Table temp;
			auto restry = table.find(sData[i].SrcNode);
			//printf("%f\n",restry->second.dist);
			temp.dist = (restry->second).dist + sData[i].Distance;  //�������
			//printf("1.2\n");
			temp.port = sData[i].SrcNode;
			//printf("1.3\n");
			//printf("%d %d",sData[i].DestNode,sData[i].SrcNode);
			table.insert({sData[i].DestNode,temp}); //�Ѹýڵ����hash��
			//printf("%d",table.find(sData[i].DestNode)->second.port);
			//printf("insert hash.\n");
		}
		else if(sData[i].DestNode != UDPPort &&sData[i].SrcNode!=sData[i].DestNode){
			int dis = (table.find(sData[i].SrcNode)->second).dist + sData[i].Distance;
			if(dis < (table.find(sData[i].DestNode))->second.dist){ //���º�ı������С��ԭ����
				//printf("2.3.1\n");
				Table temp;
				temp.dist = dis;
				//printf("2.3.2\n");
				temp.port = sData[i].SrcNode;
				temp.lastRecvTime = clock();
				table[sData[i].DestNode] = temp;
				//destnode�����1-3���룬����src��1-2���룬Ҫ��Ϸ�������2-3���� 
			}else if(dis > (table.find(sData[i].DestNode))->second.dist){
				printf("now get %d->%d,%f,must update %d->%d",sData[i].SrcNode,sData[i].DestNode,sData[i].Distance,UDPPort,sData[i].DestNode);
				auto restry = table.find(sData[i].DestNode); //�ҵ�1-3 ���룬����Ǵ��ھӷ������� 
				if((restry->second).port == sData[i].SrcNode){  //�����ʱ�����ֵ��ھӽڵ�Ҳ��һ���Ļ� 
					//�޸ĵ�ԭ����·���ĵ�һ�£���·���ϵľ��뷢���仯�����нڵ������ȣ���Ҳ��Ҫ���� 
					(table.find(sData[i].DestNode))->second.dist = dis; //��Ҫ����des������ 
					newget ++;
				}
			}
		}
	}
	//��ӡ��Ϣ ����ʾ̨
	printf("\n");
	printf("##Sent. Source Node = %d ; Senquence Number = %d\n",source,seq);
	int index = 0;
	for(auto i:table){
		printf("# %d DestNode =  %d ;  Distance = %f; Neighbor = %d\n",index,i.first,i.second.dist,i.second.port);
		index += 1;
		//printf("lastRecvTime = %d\n",i.second.lastRecvTime);
	}
	printf("\n");

	//д����Ϣ ��log.txt
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
	//������Ҫ����ʱ��
	if(table.find(des) != table.end()){
		//����ڵ��ڱ��У�����ʱ�� 
		(table.find(des)->second).lastSendTime = time;
		//printf("settime:time=%d\n",time);
	}
	return;
	
}

bool DVAlg::isSend(int des,long int time){
	//�ж��Ƿ�ʱ����Ҫ���� 
	if((table.find(des)->second).lastSendTime == -1){
		return true;
	}
	if( table.find(des) != table.end()){
		//����ڵ��ڱ���
		
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
			it = table.erase( it );		// ����һ��ִ��it++ָ����һ���ڵ�
		}
		else{
			it++;
		}
	}
	return;
}

#endif

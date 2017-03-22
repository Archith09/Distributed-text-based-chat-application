typedef struct memberDetails{
	char ip[16];
	char port[5];
	char userName[32];
	//char rxBytes[RXBYTE_BUFSIZE];
	bool operator == (const memberDetails &rhs)
	{
	    //return ((strcmp(rhs.ip,ip)==0) && (strcmp(rhs.port,port)==0) && (strcmp(rhs.userName,userName)==0) && (strcmp(rhs.rxBytes,rxBytes)==0));
	    return ((strcmp(rhs.ip,ip)==0) && (strcmp(rhs.port,port)==0) && (strcmp(rhs.userName,userName)==0));
	}

}memberDetails;

typedef struct sendData{
	char message[2048];
	int messageType;
	long sequenceNumber;
	int timestamp;
	bool operator < (const sendData &rhs)const
	{
	    return rhs.sequenceNumber < sequenceNumber;
	}
}sendData;

typedef struct leader{
	char ip[16];
	int port;
	char leaderName[32];
}LEADER;

class chat_node{
public:
	chat_node(char userName[],int entry,char ipaddr[] , int port, long recentSequenceNum );
	~chat_node();
	bool isLeader;
	//int  statusServer;
	long recentSequenceNum;
	//int entryNum;
	char ip[16];
	char memberName[32];
	//char rxBytes[RXBYTE_BUFSIZE];
	int port;
	LEADER leader;
	list<memberDetails> memberList;
	map<string,string> memberMap;
	map<string,double> checkStatus;
	list<sockaddr_in> socketList;
	//PriorityBlockingQueue<sendData> holdbackQueue;
	Queue<sendData> comfortQueue;
	//Queue<sendData> sendQueue;
	//Queue<string> printQueue;

	list<memberDetails> getMemberList(){
			list<memberDetails> copyList(memberList.begin(),memberList.end());
			return copyList;
	}
	list<sockaddr_in> getSocketList(){
		list<sockaddr_in> copySockList(socketList.begin(),socketList.end());
		return copySockList;
	}
};

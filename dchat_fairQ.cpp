#include "structures_fairQ.h"
#include "server_udp.h"

using namespace std;

Server* joinServer;
Server* chatServer;
Server* ackServer;
Server* heartbeatServerSend;
Server* heartbeatServerRecv;

int main(int argc, char* argv[]){

	char myipAddress[100];
	int myportNum;
	char userName[100];
	char leaderIpAddress[100];
	char memberIpAddress[100];
	char tempName[100];
	char tempIP[100];
	int tempPortNum;
	int memberPortNum;
	int leaderPortNum;
	string connectIP;
	string connectPort;

	strcpy(userName,argv[1]);
	
	strcpy(myipAddress,getIPaddress().c_str());
	myportNum = getPortNumber();
	
	// update global details
	string myip(myipAddress);
	string myusername(userName);
	mydetails.username = myusername;
	mydetails.ip = myip;
	mydetails.port = myportNum;


	joinServer = new Server(myipAddress,myportNum);
	chatServer = new Server(myipAddress,myportNum+1);
	heartbeatServerSend = new Server(myipAddress,myportNum+2);
	ackServer = new Server(myipAddress,myportNum+3);
	heartbeatServerRecv = new Server(myipAddress,myportNum+4);
	
	if(argc <2 || argc > 3){
		fprintf(stderr,"Sequencer: dchat Username  or Member: dchat Username 0.0.0.0:0000\n");
		exit(1);
	}
	
	if(argc==2){
		strcpy(leaderIpAddress,myipAddress);
		leaderPortNum = myportNum;
		checkLeader = true;
		mydetails.isLead = true;
		memberAddition(userName,leaderIpAddress,leaderPortNum,checkLeader);
		pthread_mutex_lock( &count_mutex );
		socketAction(leaderIpAddress,leaderPortNum,true);
		pthread_mutex_unlock( &count_mutex );
		
		strcpy(ld.name,userName);
		strcpy(ld.ip,leaderIpAddress);
		ld.port=leaderPortNum;
		currentSequenceNumber = 1;
		cout<<userName<<" started a new chat, listening on "<<leaderIpAddress<<":"<<leaderPortNum<<endl;
	 	cout<<"Succeeded, current users:"<<endl;
		cout<<userName<<" "<<leaderIpAddress<<":"<<leaderPortNum<<" (Leader) "<<endl;
		cout<<"Waiting for others to join..."<<endl;
	}
	
	if(argc==3){
		checkLeader = false;
		currentSequenceNumber = 0;
		mydetails.isLead = false;
		strcpy(memberIpAddress,myipAddress);
		memberPortNum = myportNum;
		string details = argv[2];
		size_t colon = details.find(":");
		if(colon == string::npos){
			cout<<"Please enter valid ip Address and port Number in the form of 0.0.0.0:0000"<<endl;
			exit(1);
		}
		connectIP = details.substr(0,colon);
		connectPort = details.substr(colon+1,-1);
		DATA info;
		info.message_type = JOIN;
		strcpy(info.user,userName);
		strcpy(info.ip,myipAddress);
		info.port = myportNum;
		info.isLead = false;
		DATA recvMsg;
		struct sockaddr_in clientAddress;
		bzero((char*) &clientAddress,sizeof(clientAddress));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_port = htons(atoi(connectPort.c_str()));
		clientAddress.sin_addr.s_addr = inet_addr(connectIP.c_str());

		struct timeval timeout;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
	
		if(setsockopt(joinServer->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
			perror("ERROR IN SETSOCKOPT\n");	
		}
	
		socklen_t sendlen = sizeof(clientAddress);
		if(sendto(joinServer->get_socket(),&info,sizeof(struct DATA),0,(struct sockaddr *)&clientAddress,sendlen)<0){
			perror("ERROR IN SENDING JOIN MESSAGE\n");
		}
		if(recvfrom(joinServer->get_socket(),&recvMsg,sizeof(struct DATA),0,(struct sockaddr *)&clientAddress,&sendlen)<0){
			cout<<"Sorry, no chat is active on "<<connectIP<<":"<<connectPort<<" ,try again later.\nBye."<<endl;
			exit(1);
		}
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		
		if(setsockopt(joinServer->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
			perror("ERROR IN SETSOCKOPT\n");	
		}
	
		
		if(recvMsg.message_type==INDIRECT){
			cout<<string(recvMsg.message)<<endl;
			string iptemp(recvMsg.ip);
			connectIP = iptemp;
			int p = recvMsg.port;
			connectPort = to_string(p);
			clientAddress.sin_family = AF_INET;
			clientAddress.sin_port = htons(atoi(connectPort.c_str()));
			clientAddress.sin_addr.s_addr = inet_addr(connectIP.c_str());
			
			struct timeval tv;
			tv.tv_sec = 2;
			tv.tv_usec = 0;
	
			if(setsockopt(joinServer->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&tv,sizeof(tv)) < 0){
				perror("ERROR IN SETSOCKOPT\n");	
			}
	
			
			if(sendto(joinServer->get_socket(),&info,sizeof(struct DATA),0,(struct sockaddr *)&clientAddress,sendlen)<0){
				perror("ERROR IN SENDING JOIN MESSAGE\n");
			}
			
			// recieve the user list for connecting indirectly to the sequencer. 
			if(recvfrom(joinServer->get_socket(),&recvMsg,sizeof(struct DATA),0,(struct sockaddr *)&clientAddress,&sendlen)<0){
				perror("ERROR IN RECEIVING JOIN RESPONSE");
			}
			
			
			
			tv.tv_sec = 0;
			tv.tv_usec = 0;
		
			if(setsockopt(joinServer->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&tv,sizeof(tv)) < 0){
				perror("ERROR IN SETSOCKOPT\n");	
			}
			
		}

		// update the sequence number to be 1 less than the sequencer's sequenc number;
		currentSequenceNumber = recvMsg.sequenceNum - 1;
		
		cout<<userName<<" joining a new chat on "<<connectIP<<":"<<connectPort<<", listening on "<<memberIpAddress<<":"<<memberPortNum<<endl;
		cout<<"Succeeded current users:"<<endl;

		string receiveMsg(recvMsg.message);
		vector<string> inputMessage= parseMessage(receiveMsg,"|");
		//char *inputMessage = strtok(recvMsg,"|");
		int i = 0;
		int len = inputMessage.size();
		while(i<len){
			
			
			vector<string> details = parseMessage(inputMessage[i],":");
	
			char tempIP[100];
			strcpy(tempIP,details[0].c_str());
			int tempPortNum = atoi(details[1].c_str());
			char tempName[100];
			strcpy(tempName,details[2].c_str());
			char tempStatus[100];
			strcpy(tempStatus,details[3].c_str());
			
			if(strcmp(tempStatus,"true")==0){
				cout<<string(tempName)<<" "<<string(tempIP)<<":"<<to_string(tempPortNum)<<" (Leader)"<<endl;
			}
			else{
				cout<<string(tempName)<<" "<<string(tempIP)<<":"<<to_string(tempPortNum)<<endl;
			}
		
			if(strcmp(tempStatus,"true")==0){	
				
				strcpy(ld.name,tempName);
				strcpy(ld.ip,tempIP);
				ld.port=tempPortNum;
			}
			memberAddition(tempName,tempIP,tempPortNum,checkLeader);
			pthread_mutex_lock( &count_mutex );
			socketAction(tempIP,tempPortNum,true);
			pthread_mutex_unlock( &count_mutex );
			i++;	
		}
	}		
		pthread_t send;
		pthread_t receive;
		pthread_t process;
		pthread_t print;
		pthread_t notice;
		pthread_t heartbeat_send;
		pthread_t heartbeat_recv;
		pthread_t checkfor_dead;
		pthread_create (&send,NULL,sendMessage,(void *) 0);
		pthread_create (&receive,NULL,receiveMessage,(void *) 0);
		pthread_create (&process,NULL,processMessage,(void *) 0);
		pthread_create (&print,NULL,printMessage,(void *) 0);
		pthread_create (&notice,NULL,noticeThread,(void *) 0);
		pthread_create (&heartbeat_send,NULL,heartbeatSend,(void *) 0);
		pthread_create (&heartbeat_recv,NULL,heartbeatReceive,(void *) 0);
		pthread_create (&checkfor_dead,NULL,checkForDeadMembers,(void *) 0);

		
		while(1){
			DATA toSend;
			struct sockaddr_in myAddress;
			string sendMessage;

			string enMessage;

			string chat = "";
			getline(cin,chat);
			//Additional Logic for Ctrl+D to leave the chat
			if(cin.eof() == 1){
				toSend.message_type = LEAVE;
				strcpy(toSend.user,userName);
				strcpy(toSend.ip,myipAddress);
				toSend.port = myportNum;
				toSend.isLead = checkLeader;
				struct sockaddr_in clientAddress;
				bzero((char*) &clientAddress,sizeof(clientAddress));
				clientAddress.sin_family = AF_INET;
				clientAddress.sin_port = htons(atoi(connectPort.c_str()));
				clientAddress.sin_addr.s_addr = inet_addr(connectIP.c_str());
				//Need to change the port Number when the ACK logic is added.
				if(sendto(joinServer->get_socket(),&toSend,sizeof(struct DATA),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0){
					perror("ERROR IN SENDING LEAVE MESSAGE\n");
				}
				close(joinServer->get_socket());
				close(heartbeatServerSend->get_socket());
				close(heartbeatServerRecv->get_socket());
				close(chatServer->get_socket());
				close(ackServer->get_socket());
				exit(0);
			}
			else{
				toSend.message_type = CHAT;
				chat = encrypt(chat);
				
				sendMessage = string(userName)+"::"+chat;
				strcpy(toSend.message,sendMessage.c_str());
				strcpy(toSend.user,userName);
				strcpy(toSend.ip,myipAddress);
				toSend.port = myportNum;
			
				//push onto a queue
		
				sendQueue.push(toSend);
			}
		}
		pthread_join(send,NULL);
		pthread_join(receive,NULL);
		pthread_join(process,NULL);
		pthread_join(print,NULL);
		pthread_join(notice,NULL);
		pthread_join(heartbeat_send,NULL);
		pthread_join(heartbeat_recv,NULL);
		pthread_join(checkfor_dead,NULL);
		
	}


string getIPaddress(){
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
	string address;

    getifaddrs(&ifAddrStruct);
	char k[] = "em1";

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            string name(ifa->ifa_name);
            if(name.compare("em1")==0){
            	address = addressBuffer;        	
            }
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return address;
}

int getPortNumber(){
	int port = 3000;
	if(checkPortNumber(port)){
		if(checkPortNumber(port+1)){
			if(checkPortNumber(port+2)){
				if(checkPortNumber(port+3)){
					if(checkPortNumber(port+4)){
						return port;
					}
				}
			}
		}
	}
	else{
		port = 3000 + rand() % 100;
		while( (!checkPortNumber(port)) && (!checkPortNumber(port+1)) && (!checkPortNumber(port+2)) && (!checkPortNumber(port+3)) && (!checkPortNumber(port+4)) ){
			port = 3000 + rand() % 100;
		}
	}
	return port;	
}

int checkPortNumber(int portNumber){

	char *hostname = "127.0.0.1";
	int checkSocket;
	struct sockaddr_in myAddress;
	struct hostent *server;
	int checkPort = 0;
	server = gethostbyname(hostname);
	
	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons(portNumber);
	bcopy((char *)server->h_addr,(char *)&myAddress.sin_addr.s_addr, server->h_length);

	checkSocket = socket(AF_INET,SOCK_DGRAM,0);
	if (checkSocket == -1){
		perror("ERROR IN THE SOCKET CONNECTION");
		exit(1);
	}
	if (bind(checkSocket, (struct sockaddr *)&myAddress, sizeof(myAddress))<0){
		checkPort = 0;
	}
	else {
		checkPort = 1;
	}
	close(checkSocket);
	return checkPort;
}

void memberAddition(char user[],char ip[],int port,bool isLeader){

	string username(user);
	string ipaddr(ip);
	string connectionDetails = ipaddr+":"+to_string(port);

	if(chatMembers.find(connectionDetails)==chatMembers.end()){
		pthread_mutex_lock(&chatMembers_mutex);
		chatMembers[connectionDetails].push_back(username);
		if(isLeader)
			chatMembers[connectionDetails].push_back("true");
		else
			chatMembers[connectionDetails].push_back("false");
		pthread_mutex_unlock(&chatMembers_mutex);
	}
	else{
		cout<<"Members are not added"<<endl;
	}
}

void socketAction(char ip[],int port,bool status){
	
	char addressBuffer[INET_ADDRSTRLEN];
	struct sockaddr_in memberAddress;
	void* tmpAddrPtr;
	int socklist_port;
	list<sockaddr_in>::iterator to_delete;

	bool found = false;
	
	memberAddress.sin_family = AF_INET;
	memberAddress.sin_port = htons(port);
	memberAddress.sin_addr.s_addr = inet_addr(ip);
	if(status == true){
		pthread_mutex_lock(&socketList_mutex);
		socketList.push_back(memberAddress);
		pthread_mutex_unlock(&socketList_mutex);
	}
	else{

		pthread_mutex_lock(&socketList_mutex);
		
		for(std::list<sockaddr_in>::iterator it=socketList.begin(); it!=socketList.end(); it++){
			
			bzero( addressBuffer, sizeof(addressBuffer));
			tmpAddrPtr=&(it->sin_addr);
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			socklist_port = ntohs(it->sin_port);

			if( socklist_port == port && (strcmp(addressBuffer, ip) == 0 )   ){
				to_delete = it;
				found = true;
			}
		
		}
		pthread_mutex_unlock(&socketList_mutex);
		if(found){
			pthread_mutex_lock(&socketList_mutex);
			socketList.erase(to_delete);
			pthread_mutex_unlock(&socketList_mutex);
		}
		
	}
}

void *noticeThread(void *){
	struct sockaddr_in myAddress;
	bool acked = false;
	int recv_bytes = 0;
	int retry_send = 0;
	while(1){
		DATA recvMsg;
		socklen_t recvlen = sizeof(struct sockaddr);
		if(recvfrom(joinServer->get_socket(),&recvMsg, sizeof(DATA),0,(struct sockaddr *)&myAddress,&recvlen)<0){
			perror("ERROR IN RECEIVING JOIN RESPONSE in RECEIVE THREAD");
		}
		
		if(recvMsg.message_type==JOIN && checkLeader){
			string ipaddr = string(inet_ntoa(myAddress.sin_addr));
			int port = ntohs(myAddress.sin_port);
			char tempip[100];
			int tempport;
			memberAddition(recvMsg.user,recvMsg.ip,recvMsg.port,recvMsg.isLead);
	
			strcpy(tempip,recvMsg.ip);
			tempport= recvMsg.port;
			myAddress.sin_port = htons(ntohs(myAddress.sin_port));
			string sendMap ="";
			
			pthread_mutex_lock(&chatMembers_mutex);
			for (std::map<string,vector<string>>::iterator it=chatMembers.begin(); it!=chatMembers.end(); ++it)
				sendMap = it->first+":"+it->second[0]+":"+it->second[1]+"|"+sendMap;
			
			pthread_mutex_unlock(&chatMembers_mutex);

			DATA d;
			strcpy(d.message,sendMap.c_str());
			pthread_mutex_lock( &sequenceNum_mutex );
			d.sequenceNum = currentSequenceNumber;
			pthread_mutex_unlock( &sequenceNum_mutex );
	
			if(sendto(joinServer->get_socket(),&d, sizeof(DATA),0,(struct sockaddr *)&myAddress,recvlen)<0){
				perror("ERROR IN RECEIVING JOIN RESPONSE");
			}
			
			string noticeMessage = "NOTICE "+string(recvMsg.user)+" joined on "+string(recvMsg.ip)+":"+to_string(recvMsg.port);
			DATA noticeMsg;
			strcpy(noticeMsg.message,noticeMessage.c_str());
			strcpy(noticeMsg.user,recvMsg.user);
			strcpy(noticeMsg.ip,recvMsg.ip);
			noticeMsg.port = recvMsg.port;
			noticeMsg.message_type = NOTICE;
			noticeMsg.isLead = recvMsg.isLead;
			struct sockaddr_in client;
			pthread_mutex_lock(&socketList_mutex);
			
			for (std::list<sockaddr_in>::const_iterator it = socketList.begin(), end = socketList.end(); it!= end; ++it) {
				client = *it;
				
				if(sendto(joinServer->get_socket(),&noticeMsg, sizeof(DATA),0,(struct sockaddr *)&client,sizeof(client))<0){
					perror("ERROR IN RECEIVING JOIN RESPONSE");
				}

			}
			pthread_mutex_unlock(&socketList_mutex);
			
			socketAction(tempip,tempport,true);
		}
		else if(recvMsg.message_type==JOIN && !checkLeader){
			DATA indirect;
			strcpy(indirect.message,"Contact Leader");
			indirect.message_type = INDIRECT;
			
			strcpy(indirect.ip,ld.ip);
			indirect.port = ld.port;
			strcpy(indirect.user, ld.name);

			if(sendto(joinServer->get_socket(),&indirect, sizeof(DATA),0,(struct sockaddr *)&myAddress,sizeof(myAddress))<0){
					perror("ERROR IN RECEIVING JOIN RESPONSE");
			}
		}
		//Logic for the members to leave the chat. Still additions are to be done.
		else if(recvMsg.message_type==LEAVE && checkLeader){
			
			string noticeMessage = "NOTICE "+string(recvMsg.user)+" on "+string(recvMsg.ip)+":"+to_string(recvMsg.port)+" left the chat or crashed.";
			
			DATA noticeMsg;
			strcpy(noticeMsg.message,noticeMessage.c_str());
			strcpy(noticeMsg.user,recvMsg.user);
			strcpy(noticeMsg.ip,recvMsg.ip);
			noticeMsg.port = recvMsg.port;
			noticeMsg.message_type = LEAVENOTICE;
			struct sockaddr_in client;

			//The leader will remove the member from the socket List and from the member Addition map.
			socketAction(recvMsg.ip,recvMsg.port,false);
			string deleteKey = string(recvMsg.ip)+":"+to_string(recvMsg.port);
			
			pthread_mutex_lock(&chatMembers_mutex);
			chatMembers.erase(deleteKey);
			pthread_mutex_unlock(&chatMembers_mutex);
			
			string deleteheartBeatKey = string(recvMsg.user)+":"+string(recvMsg.ip)+":"+to_string(recvMsg.port);//string(recvMsg.user)+":"+
			pthread_mutex_lock(&heartbeatChecklist_mutex);
			heartbeatChecklist.erase(deleteheartBeatKey);
			pthread_mutex_unlock(&heartbeatChecklist_mutex);
			
			pthread_mutex_lock(&messageCount_mutex);
			messageCounts.erase(deleteKey);
			pthread_mutex_unlock(&messageCount_mutex);
			
			pthread_mutex_lock(&socketList_mutex);
			for (std::list<sockaddr_in>::const_iterator it = socketList.begin(), end = socketList.end(); it!= end; ++it) {
				client = *it;
				
				if(sendto(joinServer->get_socket(),&noticeMsg, sizeof(DATA),0,(struct sockaddr *)&client,sizeof(client))<0){
					perror("ERROR IN RECEIVING JOIN RESPONSE");
				}		
			}
			pthread_mutex_unlock(&socketList_mutex);
		}
		
		else if(recvMsg.message_type==NOTICE && !checkLeader){
			
			memberAddition(recvMsg.user,recvMsg.ip,recvMsg.port,recvMsg.isLead);
			pthread_mutex_lock( &count_mutex );
			socketAction(recvMsg.ip,recvMsg.port,true);
			pthread_mutex_unlock( &count_mutex );
			printQueue.push_front(recvMsg);
		}
		else if(recvMsg.message_type==NOTICE && checkLeader){
		
			printQueue.push_front(recvMsg);
		}
		else if(recvMsg.message_type==LEAVENOTICE && !checkLeader){
			printQueue.push_front(recvMsg);

			socketAction(recvMsg.ip,recvMsg.port,false);

			string deleteKey = string(recvMsg.ip)+":"+to_string(recvMsg.port);
			pthread_mutex_lock(&chatMembers_mutex);
			chatMembers.erase(deleteKey);
			pthread_mutex_unlock(&chatMembers_mutex);
			string deleteheartBeatKey = string(recvMsg.user)+":"+string(recvMsg.ip)+":"+to_string(recvMsg.port);//string(recvMsg.user)+":"+
			pthread_mutex_lock(&heartbeatChecklist_mutex);
			heartbeatChecklist.erase(deleteheartBeatKey);
			pthread_mutex_unlock(&heartbeatChecklist_mutex);
			
			
		}
		else if(recvMsg.message_type ==LEAVENOTICE && checkLeader){
			printQueue.push_front(recvMsg);
		}
	}
}
void *sendMessage(void *){

	DATA sendMsg;
	DATA recvMsg;

	// need a leader address here.
	
	struct sockaddr_in ackAddress;
	bool acked = false;
	int recv_bytes = 0;
	int retry_send = 0;
	
	socklen_t recvlen = sizeof(ackAddress);

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	
	if	(setsockopt(ackServer->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
		perror("ERROR IN SETSOCKOPT\n");	
	}
	
	while(1){
        if(checkLeader == false){ 
			sendMsg = sendQueue.front();

			// increment port +1 because chat server is port +1
			acked = false;
			while(acked == false){ // retry if it first send fails.
				struct sockaddr_in leaderAddress = getLeaderAddrStruct();
				leaderAddress.sin_port = htons(ntohs(leaderAddress.sin_port) + 1);
				socklen_t sendlen = sizeof(leaderAddress);
				if(sendto(chatServer->get_socket(), &sendMsg, sizeof(struct DATA),0,(struct sockaddr *)&leaderAddress,sendlen)<0){
					perror("ERROR in send");
				}
			
				if((recv_bytes = recvfrom(ackServer->get_socket(),&recvMsg, sizeof(struct DATA),0,(struct sockaddr *)&ackAddress,&recvlen))<0){ // need to timeout?
					perror("ERROR receiving ACK for sent message");
					
				}

				if( recvMsg.message_type == ACK && recv_bytes > 0){
					acked = true;
					sendQueue.pop();
					retry_send = 0;
				}
			}

		}
		else if( checkLeader == true ){
			
			bool send_complete = false;
			sendMsg = sendQueue.front();
			
			sendMsg.message_type = SEQ_CHAT;

			pthread_mutex_lock( &sequenceNum_mutex );
			sendMsg.sequenceNum = currentSequenceNumber;
			pthread_mutex_unlock( &sequenceNum_mutex );

			string ipaddr(mydetails.ip);
			string connectionDetails = ipaddr +":"+to_string(mydetails.port);

			if( strcmp(sendMsg.ip, ipaddr.c_str() ) == 0 && sendMsg.port == mydetails.port  ){
				pthread_mutex_lock(& messageCount_mutex);
				if(messageCounts.find(connectionDetails)==messageCounts.end() ){
					messageCounts.insert( std::pair<string,int>(connectionDetails, 1));
				}else{
					messageCounts[connectionDetails]++;	
				}
				pthread_mutex_unlock(& messageCount_mutex);
			}

			struct sockaddr_in client;
			socklen_t recvlen = sizeof(client);
			void * tmpAddrPtr;
			char addressBuffer[INET_ADDRSTRLEN];
			
			for (std::list<sockaddr_in>::iterator it = socketList.begin(), end = socketList.end(); it!= end; ++it) {
					
				client = *it;
				// increment port +1 because chat server is port +1
				client.sin_port = htons(ntohs(client.sin_port) + 1);
				acked = false;
				
				bzero( addressBuffer, sizeof(addressBuffer));
				tmpAddrPtr=&(it->sin_addr);
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
				int socklist_port = ntohs(it->sin_port);
			
				string theiripport = string(addressBuffer)+":"+to_string(socklist_port);
				
				while(acked == false){ // retry if it first send fails.
					
					retry_send++;
					cout << "sending" << endl;
					if(sendto(chatServer->get_socket(),&sendMsg, sizeof(DATA),0,(struct sockaddr *)&client,sizeof(client))<0){
						perror("ERROR IN RECEIVING JOIN RESPONSE");
					}
						
					if((recv_bytes = recvfrom(ackServer->get_socket(),&recvMsg, sizeof(struct DATA),0,(struct sockaddr *)&ackAddress,&recvlen))<0){ // need to timeout?
						perror("ERROR receiving ACK for sent message");
					}

					if( recvMsg.message_type == ACK && recv_bytes > 0){
						acked = true;
						retry_send = 0;
					}

					if(retry_send == 5){
						cout << " detecting a failure " << endl;
						break;
					}
				}
			}
			pthread_mutex_lock( &sequenceNum_mutex );
			currentSequenceNumber++;
			pthread_mutex_unlock( &sequenceNum_mutex );
			
			sendQueue.pop();
			
		}
	}
}

void *receiveMessage(void *){
	DATA recvMsg;
	DATA ackMsg;
	ackMsg.message_type = ACK;
	struct sockaddr_in myAddress;
	socklen_t recvlen = sizeof(struct sockaddr);
	
	while(1){
		
		if(recvfrom(chatServer->get_socket(),&recvMsg, sizeof(DATA),0,(struct sockaddr *)&myAddress,&recvlen)<0){
			perror("ERROR IN RECEIVING CHAT RESPONSE IN RECEIVE THREAD");
		}
		
		if(  checkLeader == false &&  recvMsg.sequenceNum == currentSequenceNumber + 1){
			holdbackQueue.push(recvMsg);
			currentSequenceNumber++;

		}
		else if(checkLeader == true){

			string ipaddr(recvMsg.ip);
			string connectionDetails = ipaddr +":"+to_string(recvMsg.port);

			if(recvMsg.message_type == CHAT ){ // for participants message counts
				
				pthread_mutex_lock(& messageCount_mutex);
				if(messageCounts.find(connectionDetails)==messageCounts.end() ){
					messageCounts.insert( std::pair<string,int>(connectionDetails, 1) );
				}else{
					messageCounts[connectionDetails]++;	
				}
				pthread_mutex_unlock(& messageCount_mutex);
			}
			
			if(fairQMap.find(connectionDetails) == fairQMap.end()){
				queue<DATA> participant_q;
				pthread_mutex_lock(&fairQMap_mutex);
				fairQMap.insert( std::pair<string, queue<DATA> >(connectionDetails, participant_q) );
				fairQMap[connectionDetails].push(recvMsg);
				pthread_mutex_unlock(&fairQMap_mutex);
			}
			else{
				pthread_mutex_lock(&fairQMap_mutex);
				fairQMap[connectionDetails].push(recvMsg);
				pthread_mutex_unlock(&fairQMap_mutex);
			}
		
		}

		myAddress.sin_port = htons(ntohs(myAddress.sin_port)+2); // +2 because the chat server is on +1
		if(sendto(ackServer->get_socket(),&ackMsg, sizeof(DATA),0,(struct sockaddr *)&myAddress,sizeof(myAddress))<0){
			perror("ERROR IN SENDING ACK");
		}
	
	}
}

void *processMessage(void *){
	
	DATA recvMsg;
	DATA ackMsg;
	
	bool acked = false;
	int recv_bytes = 0;
	int retry_send = 0;
	struct sockaddr_in ackAddress;
	socklen_t recvlen = sizeof(ackAddress);
	
	while(1){
		
		if(checkLeader == true){

			pthread_mutex_lock(&fairQMap_mutex);
			if(fairQMap.size()>0){
				
				for( map<string, queue<DATA>>::iterator it = fairQMap.begin(); it != fairQMap.end(); it++){

					if( !( (it->second).empty() ) ){
						recvMsg = it->second.front();

						if( recvMsg.message_type == SEQ_CHAT ){
							printQueue.push(recvMsg);
							
						}
						else if(recvMsg.message_type == CHAT){
							recvMsg.message_type = SEQ_CHAT;
							sendQueue.push(recvMsg);
						}

						it->second.pop();

					}
				}
			}
			pthread_mutex_unlock(&fairQMap_mutex);
				
		}
		else if(checkLeader == false){
			recvMsg = holdbackQueue.front();
			if( recvMsg.message_type == SEQ_CHAT ){
				
				printQueue.push(recvMsg);
				holdbackQueue.pop();
			}
			
		}
	}
}


void *heartbeatSend(void *){

	ALIVE aliveMsg;
	DATA ackMsg;

	ackMsg.message_type = ACK;

	strcpy(aliveMsg.user, mydetails.username.c_str());
	strcpy(aliveMsg.ip, mydetails.ip.c_str());
	aliveMsg.port = mydetails.port;

	struct sockaddr_in client;

 	while(1){

		if(checkLeader == true){

			for (std::list<sockaddr_in>::const_iterator it = socketList.begin(), end = socketList.end(); it!= end; ++it) {
					
				client = *it;

				// increment port +4, we are sending to heartbeatRecv socket of participants
				client.sin_port = htons(ntohs(client.sin_port) + 4);
					
				if(sendto(heartbeatServerSend->get_socket(),&aliveMsg, sizeof(ALIVE),0,(struct sockaddr *)&client,sizeof(client))<0){
					perror("ERROR IN RECEIVING JOIN RESPONSE");
				}

			}
			sleep(3);
		}
		else if( checkLeader == false){

			struct sockaddr_in leaderAddress = getLeaderAddrStruct();
			leaderAddress.sin_port = htons(ntohs(leaderAddress.sin_port) + 4);

			if(sendto(heartbeatServerSend->get_socket(),&aliveMsg, sizeof(ALIVE),0,(struct sockaddr *)&leaderAddress,sizeof(client))<0){
				perror("ERROR IN RECEIVING JOIN RESPONSE");
			}
			sleep(3);
		}
	}

}

void *heartbeatReceive(void *){

	ALIVE aliveMsg;
	DATA ackMsg;
	int recv_bytes = 0;
	int retry=0;
	struct timeval end_time;
	struct timeval prev_time;
	ackMsg.message_type = ACK;

	struct sockaddr_in aliveAddress;
	socklen_t recvlen = sizeof(aliveAddress);
	
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	
	if	(setsockopt(heartbeatServerRecv->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
		perror("ERROR IN SETSOCKOPT\n");	
	}

	while(1){

		if( checkLeader == true){
			
			if((recv_bytes = recvfrom(heartbeatServerRecv->get_socket(),&aliveMsg, sizeof(struct ALIVE),0,(struct sockaddr *)&aliveAddress,&recvlen))<0){ // need to timeout?
				perror("ERROR receiving ACK for sent message");
			}

			if(recv_bytes > 0){
				if(aliveMsg.message_type == 0){ // ALIVE Messages should have message type 0
					gettimeofday(&end_time, NULL);

					string ip(aliveMsg.ip);
					string username(aliveMsg.user);
				
					int port = aliveMsg.port;
					string key = username+":"+ip+":"+to_string(port);
				
					pthread_mutex_lock(&heartbeatChecklist_mutex);
					heartbeatChecklist[key] = end_time.tv_sec;
					pthread_mutex_unlock(&heartbeatChecklist_mutex);
					
				}
			}

		}
		else if(checkLeader == false){
			
			if((recv_bytes = recvfrom(heartbeatServerRecv->get_socket(),&aliveMsg, sizeof(struct ALIVE),0,(struct sockaddr *)&aliveAddress,&recvlen))<0){ // need to timeout?
				perror("ERROR receiving Election for sent message");
			}
			if(recv_bytes>0){
				
				if(aliveMsg.message_type == ELECTION){
					
					ALIVE okMsg;
					okMsg.message_type = OK;
					
					//Send OK message back to the lower_bound
					if(sendto(heartbeatServerRecv->get_socket(),&okMsg, sizeof(ALIVE),0,(struct sockaddr *)&aliveAddress,sizeof(aliveAddress))<0){
						perror("ERROR IN RECEIVING JOIN RESPONSE");
					}

					if(highestIpPort){
						pthread_mutex_lock( &count_mutex );
						socketAction(ld.ip,ld.port,false);
						pthread_mutex_unlock( &count_mutex );
						string deleteKey = string(ld.ip)+":"+to_string(ld.port);
						
						pthread_mutex_lock(&chatMembers_mutex);
						chatMembers.erase(deleteKey);
						pthread_mutex_unlock(&chatMembers_mutex);
						
						string deleteheartBeatKey = string(ld.name)+":"+string(ld.ip)+":"+to_string(ld.port);
						pthread_mutex_lock(&heartbeatChecklist_mutex);
						heartbeatChecklist.erase(deleteheartBeatKey);
						pthread_mutex_unlock(&heartbeatChecklist_mutex);
						ALIVE leaderMsg;
						strcpy(leaderMsg.ip, mydetails.ip.c_str());
						strcpy(leaderMsg.user, mydetails.username.c_str());
						leaderMsg.port = mydetails.port;
						leaderMsg.message_type = LEADERS;
						string leaderMessage = "I am the new Leader.";
						strcpy(leaderMsg.message,leaderMessage.c_str());
						struct sockaddr_in client;
						
						//Send LEADER Message to all the members in the Socket List.
						pthread_mutex_lock(&socketList_mutex);
						for (std::list<sockaddr_in>::const_iterator it = socketList.begin(), end = socketList.end(); it!= end; ++it) {
							client = *it;
							client.sin_port = htons(ntohs(client.sin_port) + 4);
							if(sendto(heartbeatServerRecv->get_socket(),&leaderMsg, sizeof(ALIVE),0,(struct sockaddr *)&client,sizeof(client))<0){
								perror("ERROR IN RECEIVING JOIN RESPONSE");
							}
						}
						pthread_mutex_unlock(&socketList_mutex);
						checkLeader = true;	
						
						string ipport = mydetails.ip+":"+to_string(mydetails.port);
						
						pthread_mutex_lock(&chatMembers_mutex);
						chatMembers[ipport].at(0) = mydetails.username;
						chatMembers[ipport].at(1) = "true";
						pthread_mutex_unlock(&chatMembers_mutex);
						
						strcpy(ld.ip, mydetails.ip.c_str());
						strcpy(ld.name, mydetails.username.c_str());
						ld.port = mydetails.port;

						// send a bogus message to unblock the participant in the processMessage function
						DATA bogus;
						holdbackQueue.push(bogus);
					}	
				}
				else if(aliveMsg.message_type == OK){
					
					struct timeval timeout;
					timeout.tv_sec = 15;
					timeout.tv_usec = 0;
	
					if	(setsockopt(heartbeatServerRecv->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
						perror("ERROR IN SETSOCKOPT\n");	
					}
					
				}
				else if(aliveMsg.message_type == LEADERS){
					//Update the leader details;
					strcpy(ld.ip, aliveMsg.ip);
					strcpy(ld.name, aliveMsg.user);
					ld.port = aliveMsg.port;
					
					string ipport = string(aliveMsg.ip)+":"+to_string(aliveMsg.port);
					pthread_mutex_lock(&chatMembers_mutex);
					chatMembers[ipport].at(1) = "true";
					pthread_mutex_unlock(&chatMembers_mutex);
					
					retry = 0;
					// reset the timeout for heartbeats
					struct timeval timeout;
					timeout.tv_sec = 5;
					timeout.tv_usec = 0;
	
					if	(setsockopt(heartbeatServerRecv->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
						perror("ERROR IN SETSOCKOPT\n");	
					}
				}
				
				
			}
			if(recv_bytes < 0){
				//start election
				sleep(3);
				pthread_mutex_lock( &count_mutex );
				socketAction(ld.ip,ld.port,false);
				pthread_mutex_unlock( &count_mutex );

				string deleteKey = string(ld.ip)+":"+to_string(ld.port);
				
				pthread_mutex_lock(&chatMembers_mutex);
				chatMembers.erase(deleteKey);
				pthread_mutex_unlock(&chatMembers_mutex);
				
				string deleteheartBeatKey = string(ld.name)+":"+string(ld.ip)+":"+to_string(ld.port);
				pthread_mutex_lock(&heartbeatChecklist_mutex);
				heartbeatChecklist.erase(deleteheartBeatKey);
				pthread_mutex_unlock(&heartbeatChecklist_mutex);

				if(retry >= 2 ){
					removeHigherMembers();
				}

				sendElection();
				retry++;

				// reset the timeout for heartbeats
				struct timeval timeout;
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;
	
				if	(setsockopt(heartbeatServerRecv->get_socket(),SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
					perror("ERROR IN SETSOCKOPT\n");	
				}

			}
		}
	}
}

void removeHigherMembers(){
	char addressBuffer[INET_ADDRSTRLEN];
	
	void* tmpAddrPtr;
	int socklist_port;
	vector<list<sockaddr_in>::iterator> higherMembers;

	string myipport = mydetails.ip+":"+to_string(mydetails.port);

	for(std::list<sockaddr_in>::iterator it=socketList.begin(); it!=socketList.end(); it++){
				
		bzero( addressBuffer, sizeof(addressBuffer));
		tmpAddrPtr=&(it->sin_addr);
		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		socklist_port = ntohs(it->sin_port);
			
		string theiripport = string(addressBuffer)+":"+to_string(socklist_port);
					
		if( myipport.compare(theiripport) < 0  ){
			higherMembers.push_back(it);
		}
	}


	pthread_mutex_lock(&socketList_mutex);
	int i;
	for( i = 0; i < higherMembers.size(); i++){
		socketList.erase(higherMembers[i]);
	}
	pthread_mutex_unlock(&socketList_mutex);



}


void* checkForDeadMembers(void*){

	while(1){

		if(checkLeader == true){
			struct timeval t;
			gettimeofday(&t,NULL);
			pthread_mutex_lock(&heartbeatChecklist_mutex);
			for(map<string, time_t>::iterator it = heartbeatChecklist.begin();  it != heartbeatChecklist.end(); it++){
				
				if((t.tv_sec - (it->second)) > 10.0){
					
					DATA leaveMessage;
					vector<string> tokens = parseMessage(it->first,":");

					leaveMessage.message_type = LEAVE;
					strcpy(leaveMessage.user, tokens[0].c_str());
					strcpy(leaveMessage.ip, tokens[1].c_str());
					leaveMessage.port = atoi(tokens[2].c_str());

					struct sockaddr_in myAddress;
					bzero((char*) &myAddress,sizeof(myAddress));
					myAddress.sin_family = AF_INET;
					myAddress.sin_port = htons(ld.port);
					myAddress.sin_addr.s_addr = inet_addr(ld.ip);

					//Need to change the port Number when the ACK logic is added.
					if(sendto(joinServer->get_socket(),&leaveMessage,sizeof(struct DATA),0,(struct sockaddr *)&myAddress,sizeof(myAddress))<0){
						perror("ERROR IN SENDING LEAVE MESSAGE\n");
					}	
				}				
			}
			pthread_mutex_unlock(&heartbeatChecklist_mutex);

		}

		sleep(1);
	}

}

void *printMessage(void *){
	
	DATA printMsg;
	string temp = ""; //temp variable for decryption

	while(1){
		
		printMsg = printQueue.front();
		printQueue.pop();
		
		string msg(printMsg.message);

		//decryption for messages_type SEQ_CHAT
		if(printMsg.message_type == SEQ_CHAT){
			
			vector<string> tokens = parseMessage(msg,"::");
			if(tokens.size() == 2){
				temp = decrypt(tokens[1]);
				msg = tokens[0]+"::"+temp;
			}else{
				temp = decrypt("");
				msg = tokens[0]+"::"+temp;
			}
		}
		cout<<msg<<endl;
		
	}	
	
}

vector<string> parseMessage(string message,const string delim){
	
	vector<string> tokens;
	string::size_type lastPos;
	string::size_type pos;

	if(message.empty()){ return tokens;} // empty tokens...

	lastPos = message.find_first_not_of(delim, 0);
	pos = message.find_first_of(delim, lastPos);

	while(string::npos != pos || string::npos != lastPos){
		tokens.push_back( message.substr(lastPos, pos-lastPos));
		lastPos = message.find_first_not_of(delim, pos);
		pos = message.find_first_of(delim, lastPos);
	}
	return tokens;

}

struct sockaddr_in getLeaderAddrStruct(){ 

	int socklist_port;
	void * tmpAddrPtr;
    char addressBuffer[INET_ADDRSTRLEN];
	
	for( list<sockaddr_in>::iterator it = socketList.begin(); it != socketList.end(); it++){
		
		bzero( addressBuffer, sizeof(addressBuffer));
		tmpAddrPtr=&(it->sin_addr);
		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		socklist_port = ntohs(it->sin_port);

		if( socklist_port == ld.port &&  ( strcmp(ld.ip,addressBuffer) == 0 ) ){
			return *it;
		}
	}
}

bool sendElection(){
	
	bool imLeader = true;	
	
	string myipport = mydetails.ip+":"+to_string(mydetails.port);
		
	struct sockaddr_in client;	
	socklen_t sendlen = sizeof(client);
	char addressBuffer[INET_ADDRSTRLEN];
	
	void* tmpAddrPtr;
	int socklist_port;
	
	if(socketList.size() == 1){
		strcpy(ld.ip, mydetails.ip.c_str());
		strcpy(ld.name, mydetails.username.c_str());
		ld.port = mydetails.port;
					
		string ipport = mydetails.ip+":"+to_string(mydetails.port);
		pthread_mutex_lock(&chatMembers_mutex);
		chatMembers[ipport].at(1) = "true";
		pthread_mutex_unlock(&chatMembers_mutex);
		checkLeader = true;
	}
	else{
		
		pthread_mutex_lock(&socketList_mutex);
		for(std::list<sockaddr_in>::iterator it=socketList.begin(); it!=socketList.end(); it++){
				
			bzero( addressBuffer, sizeof(addressBuffer));
			tmpAddrPtr=&(it->sin_addr);
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			socklist_port = ntohs(it->sin_port);
			
			string theiripport = string(addressBuffer)+":"+to_string(socklist_port);
		
			
			if( myipport.compare(theiripport) < 0  ){
				ALIVE electionMsg;
				electionMsg.message_type = ELECTION;
				string electionMessage = "Conduct Elections";
				strcpy(electionMsg.message,electionMessage.c_str());
		
				imLeader = false;
				client = *it;
				client.sin_port = htons(ntohs(client.sin_port) + 4);
				if(sendto(heartbeatServerRecv->get_socket(),&electionMsg,sizeof(struct ALIVE),0,(struct sockaddr *)&client,sendlen)<0){
					perror("ERROR IN SENDING JOIN MESSAGE\n");
				}
					
			}
		}
		pthread_mutex_unlock(&socketList_mutex);
	}	
	
	highestIpPort = imLeader;
	return imLeader;	
	
}


void printSocketList(){
	
	char addressBuffer[INET_ADDRSTRLEN];
	void* tmpAddrPtr;
	int socklist_port;

	pthread_mutex_lock(&socketList_mutex);

	
	for(std::list<sockaddr_in>::iterator it=socketList.begin(); it!=socketList.end(); it++){
				
		bzero( addressBuffer, sizeof(addressBuffer));
		tmpAddrPtr=&(it->sin_addr);
		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		socklist_port = ntohs(it->sin_port);
			
		string theiripport = string(addressBuffer)+":"+to_string(socklist_port);
		cout << "Member: " << theiripport << endl;
	}
	pthread_mutex_unlock(&socketList_mutex);

}



//encryption and decryption
string encrypt(string Source)
{
    string Crypted = Source;

    for(int Current = 0; Current < Source.length(); Current++)
        Crypted[Current] += 3;

    return Crypted;
}

string decrypt(string Source)
{
    string Decrypted = Source;

    if(!(Source.empty())){
    	for(int Current = 0; Current < Source.length(); Current++)
        Decrypted[Current] = Decrypted[Current] - 3;

    	return Decrypted;	
    }
    else{
    	return "";
    }
    
}
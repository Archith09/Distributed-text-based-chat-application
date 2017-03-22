// #include "structures.h"
#include "structures_queue.h"

using namespace std;

int main(int argc, char* argv[]){
	
	srand(getpid());
	
	if(argc <2 || argc > 3){
		fprintf(stderr,"Sequencer: dchat Username  or Member: dchat Username 0.0.0.0:0000\n");
		exit(1);
	}
	
	if(argc==2){
		sequencer.userName = argv[1];
		sequencer.myipAddress = getIPaddress();
		sequencer.myportNum = 6320;
		sequencer.a++;
		sequencer.connectIP= sequencer.myipAddress;
		sequencer.connectPort= to_string(sequencer.myportNum);
		sequencer.checkLeader = true;
		cout<<sequencer.userName<<" started a new chat, listening on "<<sequencer.myipAddress<<":"<<sequencer.myportNum<<endl;
	 	cout<<"Succeeded, current users:"<<endl;
		cout<<sequencer.userName<<" "<<sequencer.myipAddress<<":"<<sequencer.myportNum<<" (Leader) "<<endl;
		cout<<"Waiting for others to join..."<<endl;

		initiateChat(sequencer);
		return 0;
	}
	
	if(argc==3){
		newMember.userName = argv[1];
		newMember.myipAddress = getIPaddress();
		newMember.myportNum = getPortNumber();
		// newMember.myportNum = 1234;
		newMember.a++;
		newMember.checkLeader = false;
		//cout<<"I am here in member"<<newMember.myipAddress<<"\n port number"<<newMember.myportNum;
		string connectionDetails = argv[2];
		size_t colon = connectionDetails.find(":");
		if(colon == string::npos){
			cout<<"Please enter valid ip Address and port Number in the form of 0.0.0.0:0000"<<endl;
			exit(1);
		}
		newMember.connectIP = connectionDetails.substr(0,colon);
		newMember.connectPort = connectionDetails.substr(colon+1,-1);
		//cout<<"Connect to"<<newMember.connectIP<<"at port Number"<<newMember.connectPort;
		connectChat(newMember);
		return 0;
		
	}
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
	int port = 3000 + rand() % 100;
	if(checkPortNumber(port)){
		if(checkPortNumber(port+1)){
			return port;
		}
	}
	else{
		port = 3000 + rand() % 100;
		while((!checkPortNumber(port)) && (!checkPortNumber(port+1))){
			port = 3000 + rand() % 100;
		}
	}
	return port;	
}

int checkPortNumber(int portNumber){

	char *hostname = "127.0.0.1";
	int checkSocket;
	struct sockaddr_in myAddr;
	struct hostent *server;
	int checkPort = 0;
	server = gethostbyname(hostname);
	//bzero((char*) &myAddress,sizeof(myAddress));
	
	myAddress.sin_family = AF_INET;
	myAddress.sin_port = htons(portNumber);
	//myAddress.sin_addr.s_addr = INADDR_ANY;
	bcopy((char *)server->h_addr,(char *)&myAddress.sin_addr.s_addr, server->h_length);
	//server = gethostbyname(hostname);
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

void initiateChat(Member sequencer){
		
		strcpy(memberName,sequencer.userName.c_str());
		
	//	sprintf(pNum, "%d", sequencer.myportNum);													//Changes to the code
	//	string number(pNum);

		inputList = sequencer.userName + " " + sequencer.myipAddress + ":" + sequencer.connectPort + "(Leader)";
		recvlen = sizeof(theirAddress);
		mySocket = socket(AF_INET,SOCK_DGRAM,0);						//IPPROTO_UDP
		heartbeatSocket = socket(AF_INET,SOCK_DGRAM,0);
		
		if (mySocket == -1){
			perror("ERROR IN THE SOCKET CONNECTION");
			//exit(1);//???
		}

		if (heartbeatSocket == -1){
			perror("ERROR IN THE SOCKET CONNECTION");
			//exit(1);//???
		}
		
		bzero((char*) &myAddress,sizeof(myAddress));
		myAddress.sin_family = AF_INET;
		myAddress.sin_port = htons(sequencer.myportNum);
		myAddress.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(mySocket, (struct sockaddr *)&myAddress, sizeof(myAddress))<0){
			perror("ERROR IN BIND");
			//exit(1);//????
		}

		bzero((char*) &seqheartbeatAddress,sizeof(seqheartbeatAddress));
		seqheartbeatAddress.sin_family = AF_INET;
		seqheartbeatAddress.sin_port = htons(sequencer.myportNum+1);
		seqheartbeatAddress.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(heartbeatSocket, (struct sockaddr *)&seqheartbeatAddress, sizeof(seqheartbeatAddress))<0){
			perror("ERROR IN SEQUENCER Heartbeat BIND");
			//exit(1);//????
		}		


		//while(1);
		pthread_t send;
		pthread_t input;
		pthread_t receive;
		pthread_t receive_process;
		pthread_t sendHeartbeat;
		// pthread_t receiveHeartbeat;
		pthread_create (&send,NULL,sendMessage,(void *) 0);
		pthread_create (&input,NULL,seqGetInput,(void *) 0);
		pthread_create (&receive,NULL,receiveMessage,(void *) 0);
		pthread_create (&receive_process,NULL,seqProcessMessage,(void *) 0);
		pthread_create (&sendHeartbeat,NULL,sendHeartbeatMessage,(void *) 0);
		// pthread_create (&receiveHeartbeat,NULL,receiveHeartbeat,(void *) 0);
		pthread_join(send,NULL);
		pthread_join(input,NULL);
		pthread_join(receive,NULL);
		pthread_join(sendHeartbeat,NULL);
		
		while(isSeqDead){
			return;
		}
}

void connectChat(Member newMember){
	
	strcpy(memberName,newMember.userName.c_str());
	
	newSocket = socket(AF_INET,SOCK_DGRAM,0);						////IPPROTO_UDP
	memheartbeatSocket = socket(AF_INET,SOCK_DGRAM,0);


	if (newSocket == -1){
		perror("ERROR IN THE SOCKET CONNECTION\n");
		//exit(1);//???
	}

	if (memheartbeatSocket == -1){
			perror("ERROR IN THE SOCKET CONNECTION");
			//exit(1);//???
	}


	int portNumber = atoi(newMember.connectPort.c_str());	
	bzero((char*) &leaderAddress,sizeof(leaderAddress));
	leaderAddress.sin_family = AF_INET;
	leaderAddress.sin_port = htons(portNumber);
	
	bzero((char*) &memberAddress,sizeof(memberAddress));
	memberAddress.sin_family = AF_INET;
	memberAddress.sin_port = htons(newMember.myportNum);
	memberAddress.sin_addr.s_addr = htonl(INADDR_ANY);//??

	bzero((char*) &memheartbeatAddress,sizeof(memheartbeatAddress));
	memheartbeatAddress.sin_family = AF_INET;
	memheartbeatAddress.sin_port = htons(newMember.myportNum+1);
	memheartbeatAddress.sin_addr.s_addr = htonl(INADDR_ANY);


	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 1000;

	
	if	(setsockopt(newSocket,SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
		perror("ERROR IN SETSOCKOPT\n");	
	}

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	if	(setsockopt(memheartbeatSocket,SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
		perror("ERROR IN HEARTBEAT SETSOCKOPT\n");	
	}
	
	if  (bind(newSocket, (struct sockaddr *)&memberAddress, sizeof(memberAddress))<0){
		perror("ERROR IN BIND\n");	
	}

	if  (bind(memheartbeatSocket, (struct sockaddr *)&memheartbeatAddress, sizeof(memheartbeatAddress))<0){
		perror("ERROR IN MEMBER HEARTBEAT BIND\n");	
	}
	
	if (inet_aton(newMember.connectIP.c_str(), &leaderAddress.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	
	Data seqData;
	strcpy(seqData.message,"JOIN");
	strcpy(seqData.name,newMember.userName.c_str());
	strcpy(seqData.ip,newMember.myipAddress.c_str());
	seqData.port = newMember.myportNum;
	seqData.message_type = DATA;
	
	//Internal Message
	struct communication msg;
	msg.d = seqData;
	if(sendto(newSocket,&msg,sizeof(struct communication),0,(struct sockaddr *)&leaderAddress,addrlen)<0){
		perror("ERROR IN SENDING MESSAGE\n");
	}
	
	struct communication reply;
	if(recvfrom(newSocket,&reply,sizeof(struct communication),0,(struct sockaddr *)&leaderAddress,&addrlen)<0){
		cout<<"Sorry, no chat is active on "<<newMember.connectIP<<":"<<newMember.connectPort<<" ,try again later.\nBye."<<endl;
		exit(1);
	}	

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	if(setsockopt(newSocket,SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(timeout)) < 0){
		perror("ERROR IN SETSOCKOPT\n");	
	}
	
	if(strcmp(reply.d.message,"JOIN_THROUGH_USER")==0){
		string sequencerIP(reply.d.ip);
		newMember.connectIP = sequencerIP;
		newMember.connectPort = to_string(reply.d.port);
		close(newSocket);
		close(memheartbeatSocket);
		connectChat(newMember); // call connect chat with sequencer ip and port set.
	}
	else{
		cout<<newMember.userName<<" joining a new chat on "<<newMember.connectIP<<":"<<newMember.connectPort<<", listening on "<<newMember.myipAddress<<":"<<newMember.myportNum<<endl;
		// add my details to user list...

		printChat(reply);
	}
	char details[64];
	if(recvfrom(newSocket,&details,sizeof(details),0,(struct sockaddr *)&leaderAddress,&addrlen)<0){
		perror("ERROR IN RECEIVING\n");
	}
	
	// add myself to the users list
	// strncpy(users[no_of_members].name, newMember.userName.c_str(), 24);
	// strncpy(users[no_of_members].ip, newMember.myipAddress.c_str(), 16);
	// users[no_of_members].port = newMember.myportNum;
	// no_of_members++;

	int member_count = atoi(details);

	no_of_members = member_count;

	for (int i = 0; i<member_count;i++){
		if(recvfrom(newSocket,&reply,sizeof(struct communication),0,(struct sockaddr*)&leaderAddress,&addrlen)<0){
			perror("ERROR IN RECEIVING\n");
		}
		if(strcmp(reply.d.message,":")){
			printChat(reply);
		}
	}

	
	// on connect the sequencer will update 'users' which is a participant's local list of users;
	for (int i = 0; i < member_count; i++){

		if(recvfrom(newSocket,&reply, sizeof(struct communication),0,(struct sockaddr*)&leaderAddress,&addrlen)<0){
			perror("ERROR IN RECEIVING\n");
		}

		// add the member to the list of members
		string name(reply.d.name);
		string ip(reply.d.ip);

		// cout << "::::Adding " << name << " to my user list ::::" << endl;
		addToMemberList(reply);

		// struct chatMembers * newMember = new chatMembers;
		// strncpy(newMember->name,reply.d.name,24);
		// strncpy(newMember->ip,reply.d.ip,16);
		// newMember->port = reply.d.port;

		// memberList.push_back(newMember);
		// no_of_members++;
		// strncpy(users[no_of_members].name, reply.d.name, 24);
		// strncpy(users[no_of_members].ip, reply.d.ip, 16);
		// users[no_of_members].port = reply.d.port;

		
	}
	

	// string chat;
	// cout << "-------------Users: ---------------"<< endl;

	// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
	// 	string username((*it)->name);
	// 	string ip_addr((*it)->ip);
	// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
	// }

	// cout << "-------------END: ---------------"<< endl;


	pthread_t mem_send;
	pthread_t mem_input;
	pthread_t mem_receive;
	pthread_t heartbeat_thread;
	pthread_t mem_process;
	pthread_create (&mem_send,NULL,memberSend,(void *) 0);
	pthread_create (&mem_input,NULL,memberGetInput,(void *) 0);
	pthread_create (&mem_receive,NULL,memberReceive,(void *) 0);
	pthread_create (&mem_process,NULL,memberProcessMessage,(void *) 0);
	pthread_create(&heartbeat_thread, NULL,memberRecieveHeartbeat, (void *)0);
	pthread_join(mem_send,NULL);
	pthread_join(mem_input,NULL);
	pthread_join(mem_receive,NULL);
	pthread_join(heartbeat_thread,NULL);
	
	while(isDead){
		return;
	}
		
}



void *seqGetInput(void *){
	while(1){
		
		Data toSend;
		char sendMessage[1024];
		string chat="";	
		getline(cin,chat);

		if(cin.eof()==1){
			strcpy(sendMessage, "LEAVESEQ");
			//memSend.message_type = LEAVE;
			isSeqDead = 1;
		}
		else
			strcpy(sendMessage, chat.c_str());
		// strcpy(sendMessage,chat.c_str());
		toSend.message_type = CHAT;
		strcpy(toSend.message,sendMessage);
		strcpy(toSend.name,memberName);

		struct communication cn;
		cn.d = toSend;
		
		sendQueue.push(cn);

	}
}


void *sendMessage(void *){

	while(1){

		// cout << to_string(sendQueue.size()) << endl;

		if(sendQueue.size() > 0){

			struct communication cn;

			cn = sendQueue.front();
			// cn.d = toSend;
			// cout<<sequencer.userName<<"::"<<cn.d.message<<endl;
			for(int i = 0; i< no_of_members;i++){
				if(sendto(mySocket,&cn,sizeof(struct communication),0,(struct sockaddr*)&userList[i],addrlen)<0){
					perror("ERROR IN SEND- Here 1\n");
				}
			}

			sendQueue.pop();
		}

		if(isSeqDead == 1){
			exit(0);
		}

	}
}

void *receiveMessage(void *){
	
	struct communication input;
	
	while(1){
		if(recvfrom(mySocket,&input,sizeof(struct communication),0,(struct sockaddr*)&theirAddress,&recvlen)<0){
			perror("ERROR IN RECEIVE-- Here 2\n");	
		}	
		// receiveQueue.push(input);	
		string ip_port(input.d.ip);

		ip_port += to_string(input.d.port);

		fairQMap[ip_port].push(input);
	}
}

void *seqProcessMessage(void*){

	// int start;
	// map<string, queue<struct communication>>::iterator start;
	// start = fairQMap.begin();
	queue<struct communication> participant_q;

	while(1){

		if(!fairQMap.empty()){
			for( map<string, queue<struct communication>>::iterator it = fairQMap.begin(); it != fairQMap.end(); it++){ 
				participant_q = (it->second);
				// cout << participant_q.size() << endl;
				if( participant_q.size() > 0 ) {

					// cout << "size of my map currently " << fairQMap.size() << endl;

					struct communication input;

					input = participant_q.front();

					Data sendMsg;		
					string recvMsg(input.d.message);
					string user = input.d.name;
					string sendMessage;

					// cout << " have a message from  " << user << endl;

					if(recvMsg == "JOIN"){
						sendMessage="Succeeded, current users:\n";
						sendMessage += inputList;

						//cout << " inputList " << sendMessage << endl;

						strcpy(sendMsg.message,sendMessage.c_str());
					
						struct communication cn;
						cn.d = sendMsg;
						cn.d.message_type= SELF;
						if(sendto(mySocket,&cn,sizeof(struct communication),0,(struct sockaddr*)&theirAddress,addrlen)<0){
							perror("ERROR IN SEND-- Here 2\n");	
						}
						multicast(input,theirAddress);
						printChat(input);

					

						// cout << "-------------Users: ---------------"<< endl;

						// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
						// 	string username((*it)->name);
						// 	string ip_addr((*it)->ip);
						// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
						// }

						// cout << "-------------END: ---------------"<< endl;
					}	
					else if(recvMsg == "LEAVE"){
						int userID = (int)ntohs(theirAddress.sin_port);
						removeMember(userID,user);
					}
					else{
						printChat(input);
						// cout << "numacks " << numAcks << endl;
						//struct communication cn;
						for(int i=0;i<no_of_members;i++){	
							if (sendto(mySocket, &input, sizeof(struct communication), 0 , (struct sockaddr *) &userList[i], sizeof(theirAddress))<0) {
								perror("ERROR IN SEND-- Here 3\n");	
							}
						}	
					}
					(it->second).pop();
					// start = it;
					if( (it++) == fairQMap.end() ){ // we are currently looking at the last element in the map
						it = fairQMap.begin();
					}
				}
			}
		}
	}

}



void *memberGetInput(void *){
	
	while(1){
		
		char sendMessage[1024];
		string chat="";	
		getline(cin,chat);
		
		if(cin.eof()==1){
			strcpy(sendMessage, "LEAVE");
			//memSend.message_type = LEAVE;
			isDead = 1;
		}
		else
			strcpy(sendMessage, chat.c_str());
		Data memSend;
		memSend.message_type = CHAT;
		strcpy(memSend.message,sendMessage);
		strcpy(memSend.name,memberName);

		struct communication cn;
		cn.d = memSend;

		// queue the message.
		sendQueue.push(cn);
	}
}

void *memberSend(void *){
	
	while(1){
		// cout << to_string(sendQueue.size()) << endl;
		if(sendQueue.size() > 0 ){
			struct communication cn;
			//cn.d = memSend;
			// cout<<"Here -1111"<<cn.d.message<<endl;

			cn = sendQueue.front();
	
	    	if (sendto(newSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &leaderAddress, addrlen)<0) {
          		perror("ERROR IN SEND- Here 2\n");
       		}

       		sendQueue.pop();
    	}

        if(isDead == 1)
			exit(0);
	}
	
}

void *memberReceive(void *){
	struct communication input;
	bool amILeader = false;

	while(1){
		if (recvfrom(newSocket, &input, sizeof(struct communication), 0, (struct sockaddr *) &leaderAddress, &addrlen)<0) {
			perror("ERROR IN RECEIVE- Here 1\n");
	    }
		
		receiveQueue.push(input);
		//cout<<input.d.message<<endl;
		
	}
}

void * memberProcessMessage(void * ){

	while(1){

		if(receiveQueue.size() > 0){

			struct communication input;

			input = receiveQueue.front();

			string inputMessage(input.d.message);

			//Handle messages from user connecting to us instead of sequencer
			if(inputMessage == "JOIN"){
				Data recvMsg;
				strcpy(recvMsg.message,"JOIN_THROUGH_USER");
				strcpy(recvMsg.ip, newMember.connectIP.c_str());
				recvMsg.port = atoi(newMember.connectPort.c_str());
				recvMsg.message_type = DATA;
			

				struct communication cn;
				cn.d = recvMsg;
				
				//forward the message to the sequencer so that the notice message can be multicasted
				if (sendto(newSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &leaderAddress, sizeof(leaderAddress))<0) {
					perror("ERROR IN SEND- Here 2\n");
				}

			}
			
			else if(inputMessage == "LEAVESEQ"){
				int userID = (int)ntohs(leaderAddress.sin_port);
				removeMember(userID,input.d.name);
			}
			// Sequencer sends new user messages... parse the info and add it to the user list. 
			else if(input.d.message_type == NEWUSER){

				//parse the notice mesage and then print it
				string noticeMessage(input.d.message);
				vector<string> tokens = parseNoticeMessage(noticeMessage);

				addNoticeMember(tokens);
				// cout << "number of members: "<< no_of_members << endl;

				// cout << "-------------Users: ---------------"<< endl;

				// // cout << "memberList:===== " << endl;
				// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
				// 	string username((*it)->name);
				// 	string ip_addr((*it)->ip);
				// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
				// }

				// // cout << "userList:===== " << endl;
				
				// // for(int i=0; i < no_of_members; i++){
				// // 	cout <<  << endl;
				// // }


				// cout << "-------------END: ---------------"<< endl;

				printChat(input);
			}
			else if(input.d.message_type == LEAVE){

				//remove the member from the list
				int userID = input.d.port;
				string user = input.d.name;
				// removeMember(userID,user);
				removeMemberFromList(userID, user);
				printChat(input);

			}
			else{
				printChat(input);	
			}
			receiveQueue.pop();
		}
	}

}

void *printChat(communication cn)
{
	
	if(cn.d.message_type == CHAT){
	   cout<<cn.d.name<<"::"<<cn.d.message<<endl; 
	}
	if(cn.d.message_type == SELF){
	    cout<<cn.d.message<<endl;
	}
	else{
	    cout<<cn.d.message<<endl;
	}
}

void *removeMember(int userID, string user){
	int flag = 0;
	cout << "number of members in RemoveMembers:" << to_string(no_of_members) <<endl;
	for(int i = 0; i<no_of_members;i++){
		// cout << "Removing "<< ntohs(userList[i].sin_port) << endl;

		if((int)ntohs(userList[i].sin_port) == userID){
			userList[i] = userList[i+1];
			flag = 1;
		}
		else if (flag == 1){
			userList[i] = userList[i+1];
		}
	}
	no_of_members--;
	count_of_members--;
	flag = 0;

	
	list<struct chatMembers*>::iterator temp;
	char* ip_to_remove;
	//removing from memberlist
	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		
		// string ip((*it)->ip);
		int p = (*it)->port;

		// if(( myip.compare(ip) == 0 ) && (p == memport)){
		if(p == userID){
			// string temp(((*it)->ip));
			ip_to_remove = (*it)->ip; 
			temp = it;
			break;	
		}

	}	

	delete(*temp); // free the allocated memory
	memberList.erase(temp); // remove the pointer

	// cout << "-------------Users: ---------------"<< endl;

	// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
	// 	string username((*it)->name);
	// 	string ip_addr((*it)->ip);
	// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
	// }

	// cout << "-------------END: ---------------"<< endl;

	
	Data dn;
	// dn.message_type = SELF;
	dn.message_type = LEAVE;
	strncpy(dn.name, user.c_str(), 24);
	strncpy(dn.ip, ip_to_remove, 16);
	dn.port = userID;
	string chat = "";
	chat = "NOTICE "+ user+ "left the chat or crashed";
	strcpy(dn.message,chat.c_str());
	struct communication cn;
	cn.d = dn;
	printChat(cn);

	for(int i=0;i< no_of_members;i++){	
		if (sendto(mySocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &userList[i], sizeof(theirAddress))<0) {
			perror("ERROR IN SEND-- Here 3\n");	
		}
	}
	
}

 // separate method for now just because no_of_members on participant should be 
 //   one less than the actual number, because we dont store sequencer user in the lists
void *removeMemberFromList(int userID, string user){
	int flag = 0;
	cout << "number of members in RemoveMembers:" << to_string(no_of_members) <<endl;
	// cout << "attempting to remove port " << to_string(userID) << endl;
	for(int i = 0; i<no_of_members;i++){
		
		cout << "--looking at " << ntohs(userList[i].sin_port) << endl;

		if((int)ntohs(userList[i].sin_port) == userID){

			// cout << "Found user to delete! " << endl;
			// cout << "Removing " << ntohs(userList[i].sin_port) << endl;
			userList[i] = userList[i+1];
			flag = 1;
		}
		else if (flag == 1){
			userList[i] = userList[i+1];
		}
	}
	no_of_members--;
	count_of_members--;
	flag = 0;

	// cout << " done with userList" << endl;
	
	list<struct chatMembers*>::iterator temp;
	//removing from memberlist
	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		
		// string ip((*it)->ip);
		int p = (*it)->port;

		// if(( myip.compare(ip) == 0 ) && (p == memport)){
		if(p == userID){
			cout << "Found member to delete! " << endl;
			temp = it;
			flag = 1;
			break;	
		}

	}	

	if(flag == 1){
		// cout << "preparing to delete" << endl;
		delete(*temp); // free the allocated memory
		memberList.erase(temp); // remove the pointer
	}
	else{
		cout << " did not find the user to delete in memberlist" << endl;
	}

	

	// cout << "-------------Users: ---------------"<< endl;
	// for(int i = 0; i<no_of_members;i++){
		
	// 	cout << "port " << ntohs(userList[i].sin_port) << endl;

	// }

	// cout << "-------------END: ---------------"<< endl;

	// cout << "-------------Members: ---------------"<< endl;

	// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
	// 	string username((*it)->name);
	// 	string ip_addr((*it)->ip);
	// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
	// }

	// cout << "-------------END: ---------------"<< endl;

}



void *multicast(communication input,sockaddr_in theirAddress){ // this is used for multicasting joined users
	bool isAlive = false;
	int bytes = 0;
	for(int i= 0; i < no_of_members; i++){
		if((int)ntohs(theirAddress.sin_port) == (int)ntohs(userList[i].sin_port)){
			isAlive = true;
		}
	}
	if(!isAlive){ // this indicates that a new user is joining...
		Data dn;
		// dn.message_type = SELF;
		dn.message_type = NEWUSER;
		string name(input.d.name);
		string ip(input.d.ip);
		string chat_msg = "";
		chat_msg = "NOTICE " + name + " joined on "+ ip + ":" + to_string(input.d.port);
		strcpy(dn.message,chat_msg.c_str());
		//cout<<dn.message<<endl;
		struct communication cn;
		cn.d = dn;

		for(int i=0;i<no_of_members;i++){	
			if (bytes = sendto(mySocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &userList[i], sizeof(theirAddress))<0) {
				perror("ERROR IN SEND-- Here 3\n");	
			}
			cout<<bytes<<endl;
		}
		
		printChat(cn);
		
		//add to the userList
		userList[no_of_members] = theirAddress;

		no_of_members++;
	}

	// add to users (struct ), this will be replaced by Memberlist
	// strcpy(users[count_of_members].name, input.d.name);
	// strcpy(users[count_of_members].ip, input.d.ip);
	// users[count_of_members].port = input.d.port;
	// users[count_of_members].a= (int)ntohs(theirAddress.sin_port);
	
	// add member to the user list
	struct chatMembers* chatter = new chatMembers;
	strcpy(chatter->name, input.d.name);
	strcpy(chatter->ip, input.d.ip);
	chatter->port = input.d.port;
	chatter->a = (int)ntohs(theirAddress.sin_port);
	memberList.push_back(chatter);

	string ip_port(input.d.ip);
	ip_port += to_string(input.d.port);

	queue<struct communication> participant_q;
	// checkStatus.insert( pair< string, int >( ip_port , 0 ) );
	fairQMap.insert( pair<string, queue<struct communication>> (ip_port, participant_q) );

	count_of_members++;
	cout << "Count is:" << count_of_members << endl;

	// cout << "-------------Users: ---------------"<< endl;

	// for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
	// 	string username((*it)->name);
	// 	string ip_addr((*it)->ip);
	// 	cout << username + " "+ ip_addr +":"+ to_string((*it)->port) +"\n";
	// }

	// cout << "-------------END: ---------------"<< endl;


	
	char send_message[1024];
	strcpy(send_message,"");
	char snum[5];
	sprintf(snum, "%d", count_of_members);
	strcpy(send_message,snum);   
	//cout<<"Send message is:"<<send_message;
	//string send_message = to_string(count_of_members);

	if (sendto(mySocket, &send_message, sizeof(send_message), 0 , (struct sockaddr *) &theirAddress, sizeof(leaderAddress))<0) {	//check if sizeof(leaderAddress)
				perror("ERROR IN SEND-- Here 3\n");	
	}

	struct communication count;
	// for(int i = 0; i<count_of_members;i++){
	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		
		// string name(users[i].name);
		// string ip(users[i].ip);
		string name((*it)->name);
		string ip((*it)->ip);

		string chat = "";
		// chat = name+ " "+ip+":"+ to_string(users[i].port);
		chat = name+ " "+ip+":"+ to_string((*it)->port);
		strcpy(count.d.message, chat.c_str());
		count.d.message_type = SELF;
		
		if (sendto(mySocket, &count, sizeof(struct communication), 0 , (struct sockaddr *) &theirAddress, sizeof(leaderAddress))<0) {	//check if sizeof(leaderAddress)
				perror("ERROR IN SEND-- Here 3\n");	
		}
	}

	cout << ":::SENDING USERLIST::::" << endl;

	
	struct communication m;
	Data member;

	// send all the member information, one member at a time
	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		strcpy(member.name, (*it)->name);
		strcpy(member.ip, (*it)->ip);
		member.port = (*it)->port;

		m.d = member;

		if (sendto(mySocket, &m, sizeof(struct communication), 0 , (struct sockaddr *) &theirAddress, sizeof(leaderAddress))<0) {	//check if sizeof(leaderAddress)
			perror("ERROR SENDING userlist back\n");	
		}
	}



}


void *sendHeartbeatMessage(void *){  // sequencer heartbeat code.

	Data toSend;
	char sendHeartbeat[24];
	string chat="ALIVE";	
	strcpy(sendHeartbeat,chat.c_str());
	toSend.message_type = ALIVE;
	strcpy(toSend.message,sendHeartbeat);
	strcpy(toSend.name,memberName);
		
		
	struct communication cn;
	cn.d = toSend;
	cout<<sequencer.userName<<"::"<<cn.d.message<<endl;

	struct sockaddr_in participant_addr;
	struct sockaddr_in sendheartbeat_addr;
	int newport;

	while(1){
		

		for(int i = 0; i< no_of_members;i++){

			sendheartbeat_addr = userList[i];
			newport = ntohs(userList[i].sin_port)+1;
			sendheartbeat_addr.sin_port = htons(newport);

			if(sendto(heartbeatSocket,&cn,sizeof(struct communication),0,(struct sockaddr*)&sendheartbeat_addr,addrlen)<0){
				perror("ERROR IN HEARTBEAT SEND\n");
			}
		}

		string sentmessage(cn.d.message);
		//cout << "sending " << cn.d.message << endl;	

		struct communication input;
		
		numAcks =0;

		for(int i = 0; i< no_of_members;i++){
			
			if (recvfrom(heartbeatSocket, &input, sizeof(struct communication), 0, (struct sockaddr *) &participant_addr, &addrlen)<0) {
				perror("ERROR IN Heartbeat RECEIVE\n");
    		}
    		if( input.d.message_type == ACK ){
    			// kind of 
    			numAcks ++;
    			//cout << "received " << input.d.message << endl;	
    		}

		}

		//cout << "numacks " << numAcks << endl;

		sleep(2);
	}

}


void *memberRecieveHeartbeat(void*){ // participant heartbeat
	
	struct communication input;
	struct sockaddr_in ackAddress;
	socklen_t acklen = sizeof(ackAddress);
	int recv_bytes;
	bool amILeader;

	struct sockaddr_in sendheartbeat_addr;
	int newport;

	while(1){
		// cout << "Waiting for ALIVE\n" << endl;
		bzero(&input, sizeof(input));

		if ( (recv_bytes = recvfrom(memheartbeatSocket, &input, sizeof(struct communication), 0, (struct sockaddr *) &ackAddress, &acklen) ) < 0 ) {
		// if ( (recv_bytes = recvfrom(memheartbeatSocket, &input, sizeof(struct communication), 0, (struct sockaddr *) &leaderAddress, &addrlen) ) < 0 ) {
			cout << " recvbytes " << recv_bytes << endl;
			if(recv_bytes < 0){
				perror("ERROR IN HEARTBEAT RECEIVE\n");
			}
			else{
				// cout << newMember.connectPort << endl;
				cout << "recv_bytes" << recv_bytes << endl;
				cout << "NOTICE Sequencer is dead" << endl; // " << newMember.connectIP  << ":" << newMember.connectPort << endl; 
			}
	    }

		cout << recv_bytes << endl;

		string inputMessage(input.d.message);
		// cout << "receiving " << input.d.message << endl;

		if(recv_bytes > 0 ){

			if(inputMessage == "ALIVE"){

				Data reply;
				char sendACK[24];
				string chat="ACK";	
				strcpy(sendACK,chat.c_str());
				reply.message_type = ACK;
				strcpy(reply.message,sendACK);
				strcpy(reply.name,memberName);

				struct communication cn;
				cn.d = reply;

				// cout << "sending " << cn.d.message << endl;
			
				if (sendto(memheartbeatSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &ackAddress, acklen)<0) {
				// if (sendto(memheartbeatSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &ackAddress, acklen)<0) {
					perror("ERROR IN HEARTBEAT SEND\n");
				}
			}	
			else if(inputMessage == "ELECTION"){
				
				cout << "NOTICE: Election is ongoing, checking if I am the next eligible leader..." << endl;
			
				amILeader = compareLeader();

				if(amILeader == true)
				{
					//set myself to leader
					// sequencer = newMember;
					// newMember.checkLeader = 1;
					Data reply;
					char sendLEADER[24];
					string chat="LEADER";	
					strcpy(sendLEADER,chat.c_str());
					reply.message_type = LEADER;
					strcpy(reply.message,sendLEADER);
					strcpy(reply.name,memberName);
	
					struct communication cn;
					cn.d = reply;

					for(int i=0;i<no_of_members;i++){
						
						ackAddress = userList[i];
						newport = ntohs(userList[i].sin_port)+1;
						ackAddress.sin_port = htons(newport);

						// if (sendto(memheartbeatSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &userList[i], sizeof(theirAddress))<0) {
						if (sendto(memheartbeatSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &ackAddress, sizeof(acklen))<0) {
							perror("ERROR IN SEND-- Here 3\n");	
						}
					}
				}
			}	

			else if(inputMessage == "LEADER"){
			
				// update update my copy of leader info structure
				leaderAddress.sin_port = theirAddress.sin_port;
				leaderAddress.sin_addr = theirAddress.sin_addr;

				void * tmpAddrPtr= &(theirAddress.sin_addr);
    			char addressBuffer[INET_ADDRSTRLEN];
    			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

				//convert it to a string format as well
				newMember.connectIP = addressBuffer;
				newMember.connectPort = ntohs(theirAddress.sin_port);

				amILeader = false; // reset this flag for future elections, it is possible that another participant enters with higher ip/port.
			}
		
		}
		else { // sequencer is dead, send the ELECTION Message

			//amILeader = compareLeader();
			// amILeader = true;

			//cout << "am I leader? " << amILeader << endl;

			
			// if(amILeader == 1){
			cout <<"myAddress " <<  to_string((int)ntohs(myAddress.sin_port)) << endl;
			cout << "1 address " << to_string(users[1].port) << endl;
			// cout << "0 address " << to_string(ntohs(theirAddress.sin_port)) << endl;

			// if(users[1].port  == (int)(ntohs(myAddress.sin_port)-1)){

				cout << "first member" << endl;
				Data reply;
				char sendELECTION[24];
				string chat="ELECTION";	
				strcpy(sendELECTION,chat.c_str());
				reply.message_type = ELECTION;
				strcpy(reply.message,sendELECTION);
				strcpy(reply.name,memberName);
				
				struct communication cn;
				cn.d = reply;

				cout << "number of members " << no_of_members << endl;
				cout << "number of members " << newMember.a << endl;

				// for(int i=0;i<no_of_members;i++){	
				for(int i=0;i<2;i++){

					ackAddress = userList[i];
					newport = ntohs(userList[i].sin_port)+1;
					ackAddress.sin_port = htons(newport);

					if (sendto(memheartbeatSocket, &cn, sizeof(struct communication), 0 , (struct sockaddr *) &ackAddress, acklen)<0) {
						perror("ERROR IN SEND-- Here 3\n");	
					}				
				}
			// }
		}	
	}	
}




bool compareLeader(){

	// set my ip/port combo
	// void * tmpAddrPtr= &(myAddress.sin_addr);
    char addressBuffer[INET_ADDRSTRLEN];
 //    inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

	// string my_ip_port = addressBuffer + to_string(ntohs(myAddress.sin_port)-1);
	string my_ip_port = getIPaddress() + to_string(ntohs(myAddress.sin_port)-1);

	cout << "my ip/port: " << my_ip_port << endl;

	bool isLeader= false;

	cout << "number of members: "<< newMember.a << endl;


	for(int i= 0; i < newMember.a+1; i++){

		void* tmpAddrPtr= &(userList[i].sin_addr);
		bzero(addressBuffer, sizeof(addressBuffer));
		inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

		string their_ip_port = addressBuffer + to_string(ntohs(userList[i].sin_port)-1);

		cout << "their ip/port: " << their_ip_port << endl;
		// cout << "their ip/port: " << their_ip_port << endl;

		if(my_ip_port.compare(their_ip_port) == 0){ // we matched with ourself
			//do nothing.
			cout << "I matched myself" << endl;
		}

		if( my_ip_port.compare(their_ip_port) > 0  ){
			cout << "I am leader" << endl;
			isLeader = true;
		}

		else if( my_ip_port.compare(their_ip_port) < 0  ){
			cout << "I am not leader" << endl;
			isLeader = false;
		}
	}

	if(isLeader == true){
		return true;
	}
	else{
		return false;
	}
}


vector<string> parseNoticeMessage(string message){
	
	vector<string> tokens;
	string::size_type lastPos;
	string::size_type pos;
	const string delim = " ";

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


void addToMemberList( struct communication com){

	bool exists = false;

	struct chatMembers * newMember = new chatMembers;
	strncpy(newMember->name, com.d.name,24);
	strncpy(newMember->ip, com.d.ip,16);
	newMember->port = com.d.port;

	string myip(com.d.ip);

	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		string ip((*it)->ip);
		int port = (*it)->port;

		if( ( myip.compare(ip) == 0 ) && (port == com.d.port) ){ // chat member already exists in list
			exists = true;
			break;
		}

	}

	if(exists == false){
		memberList.push_back(newMember);

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(com.d.port);
		inet_aton( myip.c_str(), &addr.sin_addr);

		userList[count_of_members] = addr;

		// we update a separate counter for the number of chatmembers in order to fill up our userListarray sequentially
		count_of_members++; 

	}

}

void addNoticeMember(vector<string> tokens){

	vector<string>::iterator it;
	bool exists = false;

	struct chatMembers * newMember = new chatMembers;
			
	it = tokens.begin();
	advance(it, 1); // second element is the username
	strncpy(newMember->name, ((*it)).c_str(), 24);

	string ip_port_part = tokens.back();
			 
	const string colon_delim = ":";
	string::size_type end_of_string = ip_port_part.size();

	string::size_type colon_pos = ip_port_part.find_first_of(colon_delim, 0);
	string myip = ip_port_part.substr(0,colon_pos);
	string port = ip_port_part.substr(colon_pos+1,end_of_string);

	strncpy(newMember->ip, myip.c_str(), 24);
	int memport = stoi(port);
	newMember->port = memport;

	for (list<chatMembers*>::iterator it = memberList.begin(); it != memberList.end(); it++) {
		
		string ip((*it)->ip);
		int p = (*it)->port;

		if(( myip.compare(ip) == 0 ) && (p == memport)){
			exists = true;
			break;	
		}

	}

	if(exists == false){
		memberList.push_back(newMember);

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(memport);
		inet_aton( myip.c_str(), &addr.sin_addr);

		userList[count_of_members] = addr;

		no_of_members++; // we only increment here because we no longer receive updates to no_of_members from sequencer after connectChat()
		count_of_members++; // update the counter so that its ready for the next new member.
	}

}
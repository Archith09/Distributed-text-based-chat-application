#include "structures.h"

void startElection(Member* curMember, Server* data, Server* ack, memberDetails user){
	
	int timeout = 0;
	struct sockaddr_in ackClient;
	char ackMessage[4];
	sendData sendMessage;
	int sockfd;
	struct timeval timeout;
	struct sockaddr_in client;

	client.sin_addr.s_addr = inet_addr(user.ip);
	client.sin_family = AF_INET;
	client.sin_port = htons(atoi(user.port));

	sendMessage.messageType = LEADER;
	strcpy(sendMessage.message,curMember->ip);
	strcpy(sendMessage.message+16,to_string(curMember->port).c_str());
	strcpy(sendMessage.message+16+5,curMember->memberName);

	sockfd = sendto(data->get_socket(),&sendMessage,sizeof(sendData),0,(struct sockaddr *)&client,(socklen_t)sizeof(struct sockaddr));
	if (sockfd == -1){
		perror("error while sending message\n");
	}

	timeout.timeout_sec = 3;
	if (setsockopt(ack->get_socket(), SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
		perror("error while setting timer on socket");
	}

	while(timeout < 3){
		if(ack->get_message(ackClient,ackMessage,sizeof(ackMessage))<0){
			perror("resending message\n");

			sockfd = sendto(data->get_socket(),&sendMessage,sizeof(sendData),0,(struct sockaddr *)&client,(socklen_t)sizeof(struct sockaddr));
			if (sockfd == -1){
				perror("error while sending message\n");
				continue;
			}
			timeout++;
		}
		else{
			if(strcmp(ackMessage,"ACK")==0)
				break;
		}
	}
}

int electionMessage(Member* curMember, Server* data, Server* ack, memberDetails user, int messageCount){
	
	struct timeval timeout;
	int sockfd
	struct sockaddr_in client;
	char sendMessage[16];
	int timeout = 0;
	struct sockaddr_in ackClient;
	char ackMessage[4];

	client.sin_addr.s_addr = inet_addr(user.ip);
	client.sin_family = AF_INET;
	client.sin_port = htons(atoi(user.port)+2);

	strcpy(sendMessage,to_string(ELECTION).c_str());

	sockfd = sendto(data->get_socket(),&sendMessage,sizeof(sendMessage),0,(struct sockaddr *)&client,(socklen_t)sizeof(struct sockaddr));
	if ( sockfd < 0){
		perror("error while sending message\n");
	}

	timeout.timeout_sec = 3;
	if (setsockopt(ack->get_socket(), SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
		perror("error while setting timer on socket");
	}

	while(timeout < 3){
		if(ack->get_message(ackClient,ackMessage,sizeof(ackMessage))<0){
			perror("resending message\n");

			sockfd = sendto(data->get_socket(),&sendMessage,sizeof(sendMessage),0,(struct sockaddr *)&client,(socklen_t)sizeof(struct sockaddr));
			if ( sockfd < 0){
				perror("error while sending message\n");
				continue;
			}
			timeout++;
		}
		else{
			messageCount++;
			if(strcmp(ackMessage,"ACK")==0)
				break;
		}
	}
	return messageCount;
}

int electNewSequencer(Member* curMember, Server* data, Server* ack){
	cout << "Election in progress..\n";
	int sockfd = 0 ;
	int messageCount = 0;
	bool isGreatest = true;

	list<memberDetails>::traverse traverse;
	list<memberDetails> memberList = curMember->getmemberList();
//#ifdef DEBUG
//	cout<<curMember->memberList.size();
//	cout<<curMember->socketList.size();
//#endif
	for(traverse = curMember->memberList.begin(); traverse != curMember->memberList.end(); ++traverse){
		struct memberDetails user = *traverse;
		if((strcmp(user.userName,curMember->leader.leaderName)==0) & (strcmp(user.ip,curMember->leader.ip)==0) & ((atoi(user.port) == curMember->leader.port)) ){
			curMember->memberList.erase(traverse);
			break;
		}
	}
	memberList = curMember->getmemberList();
	for(traverse = memberList.begin(); traverse != memberList.end(); ++traverse){
		struct memberDetails user = *traverse;
		if((atoi(user.port) != curMember->port)	|| (strcmp((user.ip),curMember->ip)!= 0)){
						isGreatest = false;
						messageCount = electionMessage(curMember, data, ack, user, messageCount);
		}
	}
	
	if((curMember->memberList.size() == 1) || (isGreatest == true) || (messageCount == 0)){
		cout << "I am the Leader\n";
		curMember->isLeader = true;
		curMember->checkStatus.clear();
		curMember->recentSequenceNum = 0;
		curMember->memberMap.erase(string(curMember->leader.ip)+":"+to_string(curMember->leader.port));
		curMember->socketList.clear();
		list<memberDetails> memberList = curMember->getmemberList();
		for(traverse = memberList.begin(); traverse != memberList.end(); ++traverse){
			memberDetails user;
			user = *traverse;
			addSocket(user.ip,atoi(user.port));
			if(!((strcmp(user.ip,curMember->ip)==0) && ((atoi(user.port) == curMember->port))) ){
				startElection(curMember,data,ack,user);
			}
		}
		strcpy(curMember->leader.ip ,curMember->ip);
		curMember->leader.port = curMember->port;
		strcpy(curMember->leader.leaderName, curMember->memberName);

		curMember->statusServer = NORMAL;
		sendData newLeaderMessage;
		string message = "Election Done. New Leader:" + string(curMember->memberName);
		strcpy(newLeaderMessage.message,message.c_str());
		newLeaderMessage.messageType = CHAT;
		curMember->comfortQueue.push(newLeaderMessage);
	}
	return sockfd;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <sstream>
#include <netdb.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <resolv.h>
#include <time.h>
#include <list>
#include <array>
#include <vector>
#include <queue>
#include <map>

// #define SELF 1
// #define CHAT 2
// #define DATA 3
// #define KEY 3

#define DATA 1
#define ALIVE 2
#define JOIN 3
#define ELECTION 4
#define LEADER 5
#define LEAVE 6
#define CHAT 7
#define SELF 8
#define ACK 9
#define NEWUSER 10

using namespace std;

int mySocket;
int heartbeatSocket;
int memheartbeatSocket;

class Data{
	public:
	
	char message[1024];
	char name[100];
	char ip[100];
	int port;
	int message_type;
	
};
struct sockaddr_in leaderAddress;
int newSocket;
socklen_t addrlen = sizeof(leaderAddress);
socklen_t recvlen;
struct sockaddr_in myAddress;
struct sockaddr_in theirAddress;
struct sockaddr_in seqheartbeatAddress;
struct sockaddr_in memheartbeatAddress;


struct sockaddr_in memberAddress;
socklen_t sendlen = sizeof(memberAddress);
string inputList;

struct communication{
	Data d;
}m;


char memberName[50];
class Member
{
	public:
		
	string userName;
	string myipAddress;
	int myportNum;
	bool checkLeader;
	string connectIP;
	string connectPort;
	int a;
	
};

Member sequencer;
Member newMember;

struct chatMembers{
	char name[24];
	char ip[16];
	int port;
	int a;
};

struct chatMembers users[15];
pthread_t heartbeatThreads[15];
int count_of_members = 0;
list<chatMembers *> memberList;

queue< struct communication > sendQueue;
queue< struct communication > receiveQueue;
queue< struct communication > holdbackQueue;

//char pNum[24];

//pthread_t send;
//pthread_t receive;

		
string getIPaddress();
int checkPortNumber(int);
int getPortNumber();
int heartbeatPortNumber();
void initiateChat(Member sequencer);
void connectChat(Member newMember);
void *sendMessage(void *);
void *seqGetInput(void *);
void *receiveMessage(void *);
void *seqProcessMessage(void*);
void *memberSend(void *);
void *memberGetInput(void *);
void *memberReceive(void *);
void *memberProcessMessage(void * );
void *printChat(communication cn);
void *multicast(communication input, sockaddr_in theirAddress);
void *removeMember(int userID, string memberName);
char *encrypt(char *buf);
char *decrypt(char *buf);
bool compareLeader();
void * memberRecieveHeartbeat(void*);
void *sendHeartbeatMessage(void *);
vector<string> parseNoticeMessage(string message);
void addToMemberList( struct communication com);
void addNoticeMember(vector<string> tokens);
void *removeMemberFromList(int userID, string user);

int isDead = 0;
int isSeqDead = 0;
int numAcks;
sockaddr_in userList[15];
int no_of_members = 0;
char *decrypt_buf;

map<string, int> checkStatus;
map< string, queue<struct communication> > fairQMap;


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
#include <queue>
#include <map>
#include <string>

#define NORMAL 1

using namespace std;

class Server
    {
		public:
                Server(const char* ipAddress, int port);
                int get_socket() const;
                int get_port() const;
                std::string get_addr() const;
				int send(struct sockaddr_in clientAddress,char *msg, size_t max_size);
                int recv(struct sockaddr_in clientAddress,char *msg, size_t max_size);
				~Server();

        private:
                int udp_socket;
                struct sockaddr_in udp_addrinfo;
				int udp_port;
                std::string udp_addr;
    };
	
class Member
{
	public:
	
	Member(char ipAddress[], int port, char name[], int checkSeqNum, long seqNum);
	~Member();
	
	char userName[100];
	char myipAddress[100];
	int myportNum;
	bool checkLeader;
	long sequenceNum;
	int checkSeq;
	string theirIP;
	string theirPort;
	int status;
};



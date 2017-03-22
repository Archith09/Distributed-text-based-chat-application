#ifndef STRUCTURES_H
#define STRUCTURES_H

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
#include <math.h>
#include <queue>
#include <map>
#include <list>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <sys/time.h>

#define JOIN 1
#define CHAT 2
#define NOTICE 3
#define INDIRECT 4
#define SEQ_CHAT 5

#define ACK 6
#define LEAVE 7
#define LEAVENOTICE 8

#define ELECTION 9
#define OK 10
#define LEADERS 11


using namespace std;

struct DATA{
	char message[2048];
	char user[100];
	char ip[100];
	int port;
	int message_type;
	int sequenceNum;
	bool isLead;
};

struct ALIVE{
  char message[100];
  char user[100];
  char ip[100];
  int port;
  int message_type;
};

struct LEADER{
	char name[100];
	char ip[100];
	int port;
}ld;

struct MEMBERDETAILS{
  string username;
  string ip;
  int port;
  bool isLead;
}mydetails;

template <typename T>
class Queue
{
 public:

  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto item = queue_.front();
    queue_.pop_front();
    mlock.unlock();

    return item;
  }

  T front(){
	  std::unique_lock<std::mutex> mlock(mutex_);
	  while (queue_.empty())
	  {
		  cond_.wait(mlock);
	  }
	  auto item = queue_.front();
	  mlock.unlock();

	  return item;
  }

  void pop(T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop_front();
  }
  bool empty()
  {
      return queue_.empty();
  }

  void push(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(item);
    mlock.unlock();
    cond_.notify_one();
  }

  void push(T&& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(std::move(item));
    mlock.unlock();
    cond_.notify_one();
  }

  void push_front(const T& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_front(item);
    mlock.unlock();
    cond_.notify_one();
  }

  void push_front(T&& item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_front(std::move(item));
    mlock.unlock();
    cond_.notify_one();
  }
  
  
  
 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

Queue<DATA>holdbackQueue;
Queue<DATA>sendQueue;
Queue<DATA>printQueue;
map<string,vector<string>> chatMembers;
map<string,time_t> heartbeatChecklist;
map<string,int>messageCounts;
list<sockaddr_in> socketList;
list<string> userList;
pthread_mutex_t count_mutex;
pthread_mutex_t socketList_mutex;
pthread_mutex_t chatMembers_mutex;
pthread_mutex_t heartbeatChecklist_mutex;
pthread_mutex_t sequenceNum_mutex;
pthread_mutex_t messageCount_mutex;
bool checkLeader = false;
bool highestIpPort = false;
int currentSequenceNumber;

void memberAddition(char user[],char ip[],int port,bool isLeader);
void socketAction(char ip[],int port,bool);
string getIPaddress();
int checkPortNumber(int);
int getPortNumber();
void *receiveMessage(void *);
void *sendMessage(void *);
void *noticeThread(void *);
void *processMessage(void *);
void *printMessage(void *);
void *heartbeatSend(void *);
void *heartbeatReceive(void *);
void* checkForDeadMembers(void*);
bool sendElection();
void printSocketList();
void removeHigherMembers();

//encryption and decryption
string encrypt(string Source);
string decrypt(string Source);

vector<string> parseMessage(string message,const string delim);
struct sockaddr_in getLeaderAddrStruct();

#endif
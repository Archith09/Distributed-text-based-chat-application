#include "server_udp.h"

#include <string.h>
#include <unistd.h>
#include<stdio.h>
#include<arpa/inet.h>

Server::Server(const char * ipAddress, int port){
	udp_addr = inet_addr(ipAddress);
	udp_port = port;
	udp_socket = socket(AF_INET,SOCK_DGRAM,0);
	if (udp_socket == -1){
		perror("ERROR IN THE SOCKET CONNECTION");
		exit(1);
	}

	udp_addrinfo.sin_family = AF_INET;
	udp_addrinfo.sin_port = htons(port);
	udp_addrinfo.sin_addr.s_addr = INADDR_ANY;

	if (bind(udp_socket, (struct sockaddr *)&udp_addrinfo, sizeof(udp_addrinfo))<0){
		perror("ERROR IN BIND");
		exit(1);
	}
}

Server::~Server()
    {
        close(udp_socket);
    }


int Server::recv(struct sockaddr_in clientAddress,char *message, size_t max_size){
		socklen_t fromlen = sizeof(struct sockaddr_in);
		int bytes_recv = recvfrom(udp_socket,message,max_size,0,(struct sockaddr *)&clientAddress, (socklen_t *)&fromlen);
		return bytes_recv;
}
	
int Server::send(struct sockaddr_in clientAddress,char *message, size_t max_size){
		socklen_t fromlen = sizeof(struct sockaddr_in);
		int bytes_send = sendto(udp_socket,message,max_size,0,(struct sockaddr *)&clientAddress,fromlen);
		return bytes_send;
}
	
int Server::get_socket() const
    {
        return udp_socket;
    }

int Server::get_port() const
    {
        return udp_port;
    }

std::string Server::get_addr() const
    {
        return udp_addr;
    }
	









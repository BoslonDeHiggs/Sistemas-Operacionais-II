#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include "serdes.hpp"

#define PORT 4000

using namespace std;

int sockfd, n;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
char buf[256];
// vector<sockaddr_in> ip_addrs;
// queue<packet> received;

void receiveDtg(){
	while(true){
		bzero(buf, 256);
		int n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &cli_addr, &clilen);
		if (n < 0) 
			printf("ERROR on recvfrom");

		string aux(buf);

		packet pkt = despkt(aux);
			
		printf("[!] <name>~ %s\n", pkt._payload);
	}
}

void openSocket(){
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		printf("ERROR opening socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);    
	 
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
		printf("ERROR on binding");

	clilen = sizeof(struct sockaddr_in);
}

int main(int argc, char *argv[])
{		
	openSocket();
	
	thread receiveThread(receiveDtg);

	receiveThread.join();
	
	close(sockfd);
	return 0;
}

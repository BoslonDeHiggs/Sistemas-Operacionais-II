#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <string>
#include <time.h>
#include "data.hpp"

#define PORT 4000

using namespace std;

char buffer[256];
int sockfd;
struct sockaddr_in serv_addr, from;
struct hostent *server;
int seqn = 0;

class Client
{
private:
	string name;

public:
	Client(string name){
		if (name.length() >= 4 && name.length() <= 20)
			this->name = name;
	}

	string getName(){
		return this->name;
	}
};

void sendTo(packet pkt){
	int n = sendto(sockfd, pkt._payload, strlen(pkt._payload), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if (n < 0) 
		printf("ERROR sendto");
}

void connect(Client client){
	char message[128];
	bzero(message, 128);
	//int n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
}

void sendMessage(){
	char message[128];
	bzero(message, 128);
	copy(&buffer[5], &buffer[133], &message[strlen(message)]);

	packet pkt;
	pkt.type = DATA;
	pkt.seqn = seqn; seqn++;
	pkt.length = strlen(buffer);
	pkt.timestamp = time(NULL);
	pkt._payload = message;

	sendTo(pkt);
}

void getCommand(Client client){
	while(true){
		cout << "[#] " << client.getName() << "~ ";
		bzero(buffer, 256);
		fgets(buffer, 256, stdin);

		string aux(buffer);

		if (aux.substr(0, 5) == "SEND "){
			sendMessage();
		}
		else if (aux.substr(0, 7) == "FOLLOW "){
			// sendFollowReq();
		}
		else {
			cout << "-> [?] Command not recognized" << endl; 
		}
	}
}

void receiveReply(){
	socklen_t length = sizeof(struct sockaddr_in);

	while(true){
		int n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
		if (n < 0)
			printf("ERROR recvfrom");

		printf("\nGot an ack: %s\n", buffer);
	}
}

void openSocket(){
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
}

void closeSocket(){
	close(sockfd);
}

int main(int argc, char *argv[])
{	
	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}
	
	server = gethostbyname(argv[2]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }	
	
	openSocket();

	Client client(argv[1]);

	connect(client);

	thread messageThread(getCommand, client);
	thread replyThread(receiveReply);

	messageThread.join();
	replyThread.join();
	
	closeSocket();

	return 0;
}
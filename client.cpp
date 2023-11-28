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

#define PORT 4000

using namespace std;

char buffer[256];
int sockfd;
struct sockaddr_in serv_addr, from;
struct hostent *server;

class Client
{
private:
	string name;

public:
	Client(string name){
		this->name = name;
	}

	string getName(){
		return this->name;
	}
};

void getMessage(Client client){

	while(true){
		cout << client.getName() << ": ";
		bzero(buffer, 256);
		fgets(buffer, 256, stdin);

		string aux(buffer);

		if (aux.substr(0, 5) == "SEND "){
			char message[128];
			bzero(message, 128);
			copy(&buffer[5], &buffer[133], &message[strlen(message)]);
			int n = sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
			if (n < 0) 
				printf("ERROR sendto");
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

int main(int argc, char *argv[])
{	
	Client client(argv[1]);

	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}
	
	server = gethostbyname(argv[2]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }	
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

	thread messageThread(getMessage, client);
	thread replyThread(receiveReply);

	messageThread.join();
	replyThread.join();
	
	close(sockfd);
	return 0;
}
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

#define PORT 4000

using namespace std;

int sockfd;
char buffer[256];
struct sockaddr_in serv_addr, from;

void getMessage(){

	while(true){
		printf("Enter the message: ");
		bzero(buffer, 256);
		fgets(buffer, 256, stdin);

		int n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		if (n < 0) 
			printf("ERROR sendto");
	}
}

void receiveReply(){
	socklen_t length = sizeof(struct sockaddr_in);
	int n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
		printf("ERROR recvfrom");

	printf("\nGot an ack: %s\n", buffer);
}

int main(int argc, char *argv[])
{
    int n;
	struct hostent *server;
	
	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);

	}
	
	server = gethostbyname(argv[1]);
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

	thread messageThread(getMessage);
	thread replyThread(receiveReply);

	messageThread.join();
	replyThread.join();
	
	close(sockfd);
	return 0;
}
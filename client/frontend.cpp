#include "frontend.hpp"

#define MULTICAST_ADDR "239.255.255.200"
#define PORT 8888

FrontEnd::FrontEnd(){
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }

    // Set up my address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to any address and specific port
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Join multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }
}

void FrontEnd::listen_multicast(){
    struct sockaddr_in their_addr;
    socklen_t addr_len;
    char buffer[BUFFSIZE];
    int numbytes;

    addr_len = sizeof(their_addr);
    if ((numbytes = recvfrom(sockfd, buffer, BUFFSIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    buffer[numbytes] = '\0';
    printf("Received packet from %s:%d\nData: %s\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), buffer);
}

void FrontEnd::call_listenThread(){
    thread listenThread(&FrontEnd::listen_multicast, this);
    listenThread.join();
}
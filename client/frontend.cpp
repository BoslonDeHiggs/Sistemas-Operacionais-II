#include "frontend.hpp"
#include <netdb.h>

#define MULTICAST_ADDR "239.255.255.200"
#define PORT 8888

FrontEnd::FrontEnd(){}

void FrontEnd::init_multicast(){
    // Create UDP socket
    if ((multicastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }

    // Set up my address
    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(PORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind to any address and specific port
    if (bind(multicastSocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Join multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }
}

void FrontEnd::listen_multicast(){
    struct sockaddr_in their_addr;
    socklen_t addr_len;
    int numbytes;
    
    while(true){
        char buffer[BUFFSIZE];
        addr_len = sizeof(their_addr);
        if ((numbytes = recvfrom(multicastSocket, buffer, BUFFSIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buffer[numbytes] = '\0';
        printf("Received packet from %s:%d\nData: %s\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), buffer);
    }
}

void FrontEnd::receive_from_server(){
    while (true){
        // Receive data
        char buffer[1024];
        ssize_t bytesRead = recv(udpSocket, buffer, sizeof(buffer), 0);

        if (bytesRead == -1) {
            print_error_msg("Error receiving data");
        }
        else{
            // Print received data
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            Packet pkt = Packet::deserialize(buffer);

            print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);

            unique_lock<mutex> lock_queue_receive(mtx_queue_receive);
				pkt_queue_receive.push(pkt);
			cv_queue_receive.notify_one();
        }
    }
}

void FrontEnd::send_to_server(){
    while(true){
        unique_lock<mutex> lock_queue_send(mtx_queue_send);
			cv_queue_send.wait(lock_queue_send, [this]() { return !pkt_queue_send.empty(); });
		Packet pkt = pkt_queue_send.front();
		pkt_queue_send.pop();

        send_pkt(pkt);
    }
}

void FrontEnd::send_pkt(Packet pkt){
    string aux = pkt.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = send(udpSocket, message, strlen(message), 0);
    if (bytesSent == -1) {
        print_error_msg("Error sending data to server");
    }
}

void FrontEnd::call_listenMulticastThread(){
    thread listenThread(&FrontEnd::listen_multicast, this);
    listenThread.detach();
}

void FrontEnd::call_receiveFromServerThread(){
    thread receiveFromServerThread(&FrontEnd::receive_from_server, this);
    receiveFromServerThread.detach();
}

void FrontEnd::call_sendToServerThread(){
    thread sendToServerThread(&FrontEnd::send_to_server, this);
    sendToServerThread.detach();
}

int FrontEnd::connect_to_udp_server(const char *ip, uint16_t port){
    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        print_error_msg("Error creating socket");
        return -100;
    }

    // Set up the server address struct
    struct hostent *server = gethostbyname(ip);
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr = *((struct in_addr *)server->h_addr); // Server IP address (loopback in this example)
    serverAddr.sin_port = htons(port);
    bzero(&(serverAddr.sin_zero), 8);

    if (connect(udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        print_error_msg("Error connecting to server");
        return -200;
    }

    return 0;
}
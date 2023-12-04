#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>

class Server{
public:
	Server(uint16_t port);

    int open_udp_connection(uint16_t port);

    void listen();

private:
	int udpSocket;
    sockaddr_in serverAddress;
};
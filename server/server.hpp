#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <fstream>

using namespace std;

class Server{
public:
	Server(uint16_t port);

    int open_udp_connection(uint16_t port);

    void init_database();

    void listen();

private:
	int udpSocket;
    sockaddr_in serverAddress;
    fstream database;
};
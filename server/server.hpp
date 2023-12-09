#pragma once

#include "../packet/packet.hpp"
#include "../database/database.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <fstream>
#include <queue>

using namespace std;

class Server{
public:
	Server(uint16_t port);

    int open_udp_connection(uint16_t port);

    void init_database();

    void listen();

    void process();

private:
	int udpSocket;
    sockaddr_in serverAddress;
    Database database;
    queue<Packet> pkts_queue;
    vector<user> connected_users;
};
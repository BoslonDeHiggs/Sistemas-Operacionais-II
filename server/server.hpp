#pragma once

#include "../packet/packet.hpp"
#include "../database/database.hpp"
#include "../format/format.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <fstream>
#include <queue>
#include <semaphore>
#include <mutex>
#include <condition_variable>

using namespace std;

typedef struct pkt_addr{
    Packet pkt;
    sockaddr_in addr;

    pkt_addr(Packet packet, sockaddr_in address) : pkt(packet), addr(address){}
}pkt_addr;

class Server{
public:
	Server(uint16_t port);

    int open_udp_connection(uint16_t port);

    void init_database();

    void send(sockaddr_in clientAddress, time_t timestamp, string clientName, string payload);

    void call_listenThread();

    void call_processThread();

    void sendPendingMessages(const string& username, const sockaddr_in& clientAddress);

private:
	int udpSocket;
    sockaddr_in serverAddress;
    Database database;
    queue<pkt_addr> pkts_queue;

    mutex mtx;
    condition_variable cv;

    void listen();

    void process();
};
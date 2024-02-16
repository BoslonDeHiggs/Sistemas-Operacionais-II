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

    void send_pkt(sockaddr_in clientAddress, time_t timestamp, string clientName, string payload);

    void call_listenThread();

    void call_listenBroadcastThread();

    void call_processThread();

    void sendPendingMessages(const string& username, const sockaddr_in& clientAddress);

    int get_socket();

private:
	int udpSocket, broadcastSocket;
    sockaddr_in serverAddress, broadcastAddr;
    Database database;
    queue<pkt_addr> pkts_queue;

    mutex mtx;
    condition_variable cv;

    int create_broadcast_socket();

    void listen();

    void listen_broadcast();

    void process();

    void handleLogin(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleNewAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExistingAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleSend(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleFollow(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExit(const Packet& pkt, const sockaddr_in& clientAddress);
};
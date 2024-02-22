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
#include <netdb.h>

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

    void send_pkt(uint16_t type, sockaddr_in clientAddress, time_t timestamp, string name, string payload);

    void send_broadcast_pkt(uint16_t type, time_t timestamp, string name, string payload);

    void call_listenThread();

    void call_listenBroadcastThread();

    void call_processThread();

    void call_processBroadcastThread();

    void call_heartbeatThread();

    void sendPendingMessages(const string& username, const sockaddr_in& clientAddress);

    int get_socket();

    //ALGORITMO DE ELEICAO
    //time_t lastHeartbeat = 0;
    //const int HEARTBEAT_TIMEOUT = 10; // Timeout em segundos

private:
	int udpSocket, broadcastSocket;
    sockaddr_in serverAddress, broadcastAddr;
    Database database;
    queue<pkt_addr> pkts_queue, pkts_queue_broadcast;
    map<time_t, sockaddr_in> serverList;
    time_t id;
    bool leader;

    mutex mtx, mtx_broadcast;
    condition_variable cv, cv_broadcast;

    string get_own_address(int sockfd);

    int create_broadcast_socket();

    void listen();

    void listen_broadcast();

    void process();

    void process_broadcast();

    void heartbeat();

    void handleLogin(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleNewAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExistingAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleSend(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleFollow(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExit(const Packet& pkt, const sockaddr_in& clientAddress);
};
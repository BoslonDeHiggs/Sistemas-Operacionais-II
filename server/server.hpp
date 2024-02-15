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

class Server{
public:
	Server(uint16_t port);

    int open_udp_connection(uint16_t port);

    void init_database();

    void send_pkt(sockaddr_in clientAddress, time_t timestamp, string clientName, string payload);

    void call_listenThread();

    void call_processThread();

    void call_sendThread();
    
    virtual void call_listenThread_multicast();

    virtual void call_processThread_multicast();

    void sendPendingMessages(const string& username, const sockaddr_in& clientAddress);

    int get_socket();
    
    void pushSendQueue(const Packet& pkt, const sockaddr_in& clientAddress);

    //Tentativa de multicast
    void setup_multicast(uint16_t port);
    void send_multicast(const std::string& message);
    void send_multicast_initial_message();
    void listen_multicast();
    virtual void process_multicast();
    int multicastSocket;
    struct sockaddr_in multicastAddr;
    virtual void process();

protected:
    queue<pkt_addr> pkts_queue_listen_process, pkts_queue_process_send, pkts_queue_listen_process_multicast;

    mutex mtx_listen_process, mtx_process_send, mtx_listen_process_multicast;
    condition_variable cv_listen_process, cv_process_send, cv_listen_process_multicast;
    sockaddr_in serverAddress;
    Database database;

    void handleLogin(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleNewAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExistingAccount(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleSend(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleFollow(const Packet& pkt, const sockaddr_in& clientAddress);

    void handleExit(const Packet& pkt, const sockaddr_in& clientAddress);

private:
	int udpSocket;

    void listen();

    void send();


};
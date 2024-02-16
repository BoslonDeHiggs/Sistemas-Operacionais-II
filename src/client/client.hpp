#pragma once

#include "../format/format.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <csignal>

#define BUFFER_SIZE 256

using namespace std;

class Client{
public:
    Client(string name, const char *ip, uint16_t port);
    
    int connect_to_udp_server(const char *ip, uint16_t port);

    void login();

    void call_sendThread();

    void call_listenThread();

    void sendExit();

    ~Client();

    static void signalHandler(int signal);

private:
    string name;

    int udpSocket, broadcastSocket;
    sockaddr_in serverAddress, broadcastAddr;

    int create_udp_socket();

    int create_broadcast_socket();

    string get_own_address(int sockfd);

    void send_pkt(uint16_t code, string msg_in);

    void send_broadcast_pkt(uint16_t code, string payload);

    void get_input();

    void listen();
};

extern Client* globalClientPointer;
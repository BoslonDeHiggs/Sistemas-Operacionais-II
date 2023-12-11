#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

#define BUFFER_SIZE 256

using namespace std;

class Client{
public:
    Client(string input);
    
    int connect_to_udp_server(const char *ip, uint16_t port);

    void login();

    void follow(string username);

    void call_sendThread();

    void call_listenThread();

private:
    struct client{
        string name;
    }c_info;

    int udpSocket;
    sockaddr_in serverAddress;

    void send_pkt(uint16_t code, string msg_in);

    void get_input();

    void listen();
};
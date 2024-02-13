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
#define M_BUFFER 256

using namespace std;

class Client{
public:
    Client(string input);
    
    int connect_to_udp_server(const char *ip, uint16_t port);

    void login();

    void follow(string username);

    void send_message(string msg);

    void call_sendThread();

    void call_listenThread();

    void sendExit();

    ~Client();

    static void signalHandler(int signal); //Teste incial de encerrar sessao

private:
    struct client{
        string name;
    }c_info;

    int multicastSocket;
    int udpSocket;
    sockaddr_in serverAddress;
    sockaddr_in multicastAddress;

    void init_frontend();

    void listen_multicast();

    void call_listenMulticastThread();

    void send_pkt(uint16_t code, string msg_in);

    void get_input();

    void listen();
};

extern Client* globalClientPointer;
//bool Client::encerrarSessao = false;
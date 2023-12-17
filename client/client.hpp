#pragma once

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
    Client(string input);
    
    int connect_to_udp_server(const char *ip, uint16_t port);

    void login();

    //static void exit(int signal); //Teste inicial de encerramento de sessao

    void follow(string username);

    void call_sendThread();

    void call_listenThread();

    void sendExit();

    ~Client();

    static void signalHandler(int signal); //Teste incial de encerrar sessao

    //static bool encerrarSessao;

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

extern Client* globalClientPointer;
//bool Client::encerrarSessao = false;
#pragma once

#include "../format/format.hpp"
#include "frontend.hpp"
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
    FrontEnd frontend;

    Client(string input);

    void login();

    void follow(string username);

    void send_message(string msg);

    void call_sendThread();

    void call_listenThread();

    void sendExit();

    void init();

    ~Client();

    static void signalHandler(int signal); //Teste incial de encerrar sessao

private:
    struct client{
        string name;
    }c_info;

    int udpSocket;
    sockaddr_in serverAddress;

    void get_input();

    void listen();
};

extern Client* globalClientPointer;
//bool Client::encerrarSessao = false;
#pragma once

#include "../packet/packet.hpp"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#define BUFFSIZE 256

using namespace std;

class FrontEnd{
public:
    FrontEnd();

    queue<Packet> pkt_queue;
    queue<sockaddr_in> addr_queue;

    void call_listenThread();

private:
    int sockfd;
    struct sockaddr_in addr;

    void listen_multicast();
};
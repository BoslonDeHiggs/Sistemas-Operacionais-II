#pragma once

#include "../packet/packet.hpp"
#include "../format/format.hpp"
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

    queue<Packet> pkt_queue_send, pkt_queue_receive;
    mutex mtx_queue_send, mtx_queue_receive;
    condition_variable cv_queue_send, cv_queue_receive;

    void call_listenMulticastThread();

    void call_receiveFromServerThread();

    void call_sendToServerThread();

    int connect_to_udp_server(const char *ip, uint16_t port);

    void init_multicast();

private:
    int udpSocket, multicastSocket;
    sockaddr_in myAddr, serverAddr;
    queue<sockaddr_in> addr_queue;

    void listen_multicast();

    void send_to_server();

    void receive_from_server();

    void send_pkt(Packet pkt);
};
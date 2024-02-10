#pragma once

#include "packet.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
class CommunicationManager{
    public:
        static int receive_packet(int socket, char* buffer, struct sockaddr_in *addr);
        static void send_packet(int socket, sockaddr_in dst_address, int type, time_t timestamp, string src_name, string payload);
};    

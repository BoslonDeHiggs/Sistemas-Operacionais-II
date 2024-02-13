#include "server/server.hpp"
#include "packet/packet.hpp"
#include <iostream>
#include <cstdlib>

#define MULTICAST_PORT 5000

int main(int argc, char* argv[]){

    if (argc != 2) {
        std::cerr << "Modo de uso: ./pea_server <port>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0) {
        std::cerr << "Porta invalida." << std::endl;
        return 1;
    }

    Server server(MULTICAST_PORT);
    server.init_database();
    server.open_udp_connection((uint16_t)port);
    server.call_listenThread();
    server.call_processThread();
    server.call_sendThread();

    return 0;
}
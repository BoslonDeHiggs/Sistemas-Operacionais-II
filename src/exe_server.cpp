#include "server/server.hpp"
#include "packet/packet.hpp"

int main(int argc, char* argv[]){

    if(argc != 2){
        print_error_msg("./server <server PORT>");
        return 0;
    }

    uint16_t port = atoi(argv[1]);

    Server server(port);

    return 0;
}
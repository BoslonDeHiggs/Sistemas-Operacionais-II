#include "server/server.hpp"
#include "packet/packet.hpp"

#define PORT 4000

int main(){
    Server server(PORT);
    server.open_udp_connection(PORT);
    server.listen();

    return 0;
}
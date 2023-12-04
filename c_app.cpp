#include "client/client.hpp"

#define PORT 4000

int main(){
    Client client("Bolson");
    client.connect_to_udp_server("127.0.0.1", PORT);
    client.call_sendThread();

    return 0;
}
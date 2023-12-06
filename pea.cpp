#include "client/client.hpp"

#define PORT 4000

int main(int argc, char* argv[]){
    // cout << argc << endl;
    // cout << argv[1] << endl;

    if(argc != 3){
        cout << "[!] ERROR~ Invalid arguments" << endl;
        return 0;
    }

    Client client(argv[1]);
    client.connect_to_udp_server(argv[2], PORT);
    client.login();
    client.call_sendThread();

    return 0;
}
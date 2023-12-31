#include "client/client.hpp"

#define PORT 4000

int main(int argc, char* argv[]){

    if(argc != 3){
        cout << "[!] ERROR~ Invalid arguments" << endl;
        return 0;
    }

    string name(argv[1]);

    if(name.length() < 4 || name.length() > 20){
        cout << "[!] ERROR~ Name must be between 4 and 20 characters long" << endl;
        return 0;
    }

    Client client(name);
    //globalSession = &client;
    client.connect_to_udp_server(argv[2], PORT);
    client.login();

    /* signal(SIGINT, Client::exit); */

    client.call_listenThread();
    client.call_sendThread();

    return 0;
}
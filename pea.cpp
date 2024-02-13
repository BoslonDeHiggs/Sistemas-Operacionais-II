#include "client/client.hpp"

#define PORT 4000

int main(int argc, char* argv[]){

    if(argc != 3){
        print_error_msg("Invalid arguments");
        return 0;
    }

    string name(argv[1]);

    if(name.length() < 4 || name.length() > 20){
        print_error_msg("Name must be between 4 and 20 characters long");
        return 0;
    }
    else if(name.find("@") != std::string::npos || name.find(":") != std::string::npos || name.find(";") != std::string::npos){
        print_error_msg("Name must not contain this characters: \'@\' \':\' \';\'");
        return 0;
    }

    Client client(name);
    //globalSession = &client;
    client.connect_to_udp_server(argv[2], PORT);
    client.login();

    client.call_listenThread();
    client.call_sendThread();

    return 0;
}
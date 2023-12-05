#include "client.hpp"
#include "../packet/packet.hpp"

using namespace std;

Client::Client(string name){
    this->c_info.name = name;
}

int Client::connect_to_udp_server(const char *ip, uint16_t port){
    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -100;
    }

    // Set up the server address struct
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip); // Server IP address (loopback in this example)
    serverAddress.sin_port = htons(port);
    bzero(&(serverAddress.sin_zero), 8);

    return 0;
}

void Client::send(string payload){
    Packet packet(200, 30, payload.length(), time(NULL), payload);
    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent == -1) {
        std::cerr << "Error sending data to server" << std::endl;
    }
}

void Client::get_input(){
    while (true){
        char message[BUFFER_SIZE];
        fgets(message, BUFFER_SIZE, stdin);
        string msg = message;
        if(msg.size() > MSG_SIZE){
            cerr << "Message must not be longer than 128 characters" << endl;
        }
        else{
            send(msg);
        }
    }
}

void Client::call_sendThread(){
    thread sendThread(&Client::get_input, this);
    sendThread.join();
}
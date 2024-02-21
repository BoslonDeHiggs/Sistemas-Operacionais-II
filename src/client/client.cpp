#include "client.hpp"
#include "../packet/packet.hpp"
#include <netdb.h>

#define BROADCAST_PORT 16384

using namespace std;

Client* globalClientPointer = nullptr;

Client::Client(string name){
    this->name = "@" + name;
    globalClientPointer = this;
    std::signal(SIGINT, Client::signalHandler);
    create_udp_socket();

    call_listenThread();
    send_broadcast_pkt(DISCOV_MSG, get_own_address(udpSocket));
    call_sendThread();
}

Client::~Client() {
    globalClientPointer = nullptr;
}

string Client::get_own_address(int sockfd){
    sockaddr_in sockname;
    socklen_t socklen = sizeof(sockname);

    // Retrieve the socket address information
    if (getsockname(sockfd, reinterpret_cast<sockaddr*>(&sockname), &socklen) == -1) {
        // Handle error
        // For example, throw an exception or return an empty string
        return "";
    }

    // Convert the IP address to a string
    char buffer[INET_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET, &sockname.sin_addr, buffer, sizeof(buffer));
    if (p == nullptr) {
        // Handle error
        // For example, throw an exception or return an empty string
        return "";
    }

    // Convert port to string
    uint16_t port = ntohs(sockname.sin_port);

    // Return the IP address and port as a string
    return std::string(p) + ":" + std::to_string(port);
}

int Client::create_udp_socket(){
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        print_error_msg("Error creating socket");
        return -100;
    }

    enable_broadcast();

    return 0;
}

int Client::enable_broadcast() {
    // Enable broadcast option
    int broadcastEnable = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
        print_error_msg("Error setting broadcast option");
        close(udpSocket);
        return -200;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // Broadcasting to all interfaces
    broadcastAddr.sin_port = htons(BROADCAST_PORT);

    return 0;
}

void Client::login(){
    Client::send_pkt(LOGIN, "Request for login");
}

void Client::send_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent == -1) {
        print_error_msg("Error sending data to server");
    }
}

void Client::send_broadcast_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    if (bytesSent < 0) {
        print_error_msg("Error sending broadcast message");
    }
}

void Client::get_input(){
    while (true){
        char input[BUFFER_SIZE];
        
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            print_client_msg("Terminating session (Captured ctrl+D)");
            if (globalClientPointer != nullptr) {
                globalClientPointer->sendExit();
            }
            exit(0); // Use um código de saída apropriado
        }
        string msg = input;

        time_t timestamp = time(NULL);

        stringstream tokenizer(msg);
        string code;

        getline(tokenizer, code, ' ');
        getline(tokenizer, msg);

        if(msg.size() > MSG_SIZE){
            print_error_msg("Message must not be longer than 128 characters");
        }
        else{
            if(code == "SEND"){
                send_pkt(SEND, msg);
                print_send_msg(timestamp, this->name, code, msg);
            }
            else if(code == "FOLLOW"){
                send_pkt(FOLLOW, msg);
                print_send_msg(timestamp, this->name, code, msg);
            }
            else{
                print_error_msg("Command not valid");
            }
        }
    }
}

void Client::listen(){
    while (true){
        // Receive data
        char buffer[1024];
        socklen_t serverAddressLenght = sizeof(serverAddress);
		ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &serverAddressLenght);

        if (bytesRead == -1) {
            print_error_msg("Error receiving data");
        }
        else{
            // Print received data
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            Packet pkt = Packet::deserialize(buffer);

            if(pkt.type == DISCOV_MSG){
                login();
            }

            print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
        }
    }
}

void Client::call_sendThread(){
    thread sendThread(&Client::get_input, this);
    sendThread.join();
}

void Client::call_listenThread(){
    thread listenThread(&Client::listen, this);
    listenThread.detach();
}

void Client::sendExit(){
    Packet packet(EXIT, 0, 0, time(NULL), this->name, "Terminating session");
    string message = packet.serialize();
    ssize_t bytesSent = sendto(udpSocket, message.c_str(), message.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent == -1) {
        print_error_msg("Error sending exit message to server");
    }
}

void Client::signalHandler(int signal) {    //Teste inicial de encerrar sessao
    print_client_msg("Terminating session (Captured ctrl+C)");
    if (globalClientPointer != nullptr) {
        globalClientPointer->sendExit();
    }

    exit(signal);
}

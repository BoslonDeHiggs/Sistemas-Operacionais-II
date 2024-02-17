#include "client.hpp"
#include "../packet/packet.hpp"
#include <netdb.h>

#define BROADCAST_PORT 16384

using namespace std;

Client* globalClientPointer = nullptr;

Client::Client(string name, const char *ip, uint16_t port){
    this->name = "@" + name;
    globalClientPointer = this;
    std::signal(SIGINT, Client::signalHandler);
    create_udp_socket();
    create_broadcast_socket();

    send_broadcast_pkt(DISCOV_MSG, get_own_address(udpSocket));
    connect_to_udp_server(ip, port);
    call_listenThread();
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

    return 0;
}

int Client::create_broadcast_socket() {
    broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSocket == -1) {
        print_error_msg("Error creating broadcast socket");
        return -100;
    }

    // Enable broadcast option
    int broadcastEnable = 1;
    if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
        print_error_msg("Error setting broadcast option");
        close(broadcastSocket);
        return -200;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // Broadcasting to all interfaces
    broadcastAddr.sin_port = htons(BROADCAST_PORT);

    return 0;
}

int Client::connect_to_udp_server(const char *ip, uint16_t port){

    // Set up the server address struct
    struct hostent *server = gethostbyname(ip);
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr = *((struct in_addr *)server->h_addr); // Server IP address (loopback in this example)
    serverAddress.sin_port = htons(port);
    bzero(&(serverAddress.sin_zero), 8);

    if (connect(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        print_error_msg("Error connecting to server");
        return -200;
    }

    return 0;
}

void Client::login(){
    Client::send_pkt(LOGIN, "Request for login");
}

void Client::send_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = send(udpSocket, message, strlen(message), 0);
    if (bytesSent == -1) {
        print_error_msg("Error sending data to server");
    }
}

void Client::send_broadcast_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(broadcastSocket, message, strlen(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    if (bytesSent < 0) {
        print_error_msg("Error sending broadcast message");
    }
}

void Client::get_input(){
    
    login();

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
        ssize_t bytesRead = recv(udpSocket, buffer, sizeof(buffer), 0);

        if (bytesRead == -1) {
            print_error_msg("Error receiving data");
        }
        else{
            // Print received data
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            Packet pkt = Packet::deserialize(buffer);

            print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
        }
    }
}

void Client::listen_broadcast() {
    while (true) {
		char buffer[1024];
		socklen_t broadcastAddrLenght = sizeof(broadcastAddr);

        ssize_t bytesRead = recvfrom(broadcastSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&broadcastAddr, &broadcastAddrLenght);

        if (bytesRead == -1) {
            print_error_msg("Error receiving broadcast message");
        } else {
            buffer[bytesRead] = '\0'; // Null-terminate the received data

			Packet pkt = Packet::deserialize(buffer);

			char serverIp[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &broadcastAddr.sin_addr, serverIp, INET_ADDRSTRLEN);
			int serverPort = ntohs(broadcastAddr.sin_port);

			struct sockaddr_in responseAddress;
			memset(&responseAddress, 0, sizeof(responseAddress));
			responseAddress.sin_family = AF_INET;
			responseAddress.sin_port = htons(serverPort);
			if (inet_pton(AF_INET, serverIp, &responseAddress.sin_addr) <= 0) {
				perror("inet_pton");
			}
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
    ssize_t bytesSent = send(udpSocket, message.c_str(), message.length(), 0);
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

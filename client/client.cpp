#include "client.hpp"
#include "../packet/packet.hpp"
#include <netdb.h>

using namespace std;

//int globalUdpSocket; //Variavel global para armazenar udpSocket
//Client* globalSession = nullptr;
Client* globalClientPointer = nullptr;

Client::Client(string input){
    this->c_info.name = "@" + input;

    //globalUdpSocket = udpSocket;
    globalClientPointer = this;
    std::signal(SIGINT, Client::signalHandler); //Teste inicial para encerrar sessao
}

Client::~Client() {
    globalClientPointer = nullptr;
}

int Client::connect_to_udp_server(const char *ip, uint16_t port){
    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        print_error_msg("Error creating socket");
        return -100;
    }

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

void Client::follow(string username){
    Client::send_pkt(FOLLOW, username);
}
void Client::send_message(string msg){
    Client::send_pkt(SEND, msg);
}
void Client::send_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->c_info.name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = send(udpSocket, message, strlen(message), 0);
    if (bytesSent == -1) {
        print_error_msg("Error sending data to server");
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
                print_send_msg(timestamp, this->c_info.name, code, msg);
            }
            else if(code == "FOLLOW"){
                send_pkt(FOLLOW, msg);
                print_send_msg(timestamp, this->c_info.name, code, msg);
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

void Client::call_sendThread(){
    thread sendThread(&Client::get_input, this);
    sendThread.join();
}

void Client::call_listenThread(){
    thread listenThread(&Client::listen, this);
    listenThread.detach();
}

void Client::sendExit(){
    Packet packet(EXIT, 0, 0, time(NULL), this->c_info.name, "Terminating session");
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

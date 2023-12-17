#include "client.hpp"
#include "../packet/packet.hpp"
#include <netdb.h>

using namespace std;

Client::Client(string input){
    this->c_info.name = "@" + input;
}

int Client::connect_to_udp_server(const char *ip, uint16_t port){
    // Create a UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        std::cerr << "[!] ERROR~ Error creating socket" << std::endl;
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
        std::cerr << "[!] ERROR~ Error connecting to server" << std::endl;
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

void Client::send_pkt(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->c_info.name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = send(udpSocket, message, strlen(message), 0);
    if (bytesSent == -1) {
        std::cerr << "[!] ERROR~ Error sending data to server" << std::endl;
    }
}

void Client::get_input(){
    while (true){
        char input[BUFFER_SIZE];
        fgets(input, BUFFER_SIZE, stdin);
        string msg = input;
        
        cout << "\033[F\033[K[#] " << this->c_info.name << "~ " << msg << "\033[E";
        
        stringstream tokenizer(msg);
        string code;

        getline(tokenizer, code, ' ');
        getline(tokenizer, msg);

        if(msg.size() > MSG_SIZE){
            cerr << "[!] ERROR~ Message must not be longer than 128 characters" << endl;
        }
        else{
            if(code == "SEND"){
                send_pkt(SEND, msg);
            }
            else if(code == "FOLLOW"){
                send_pkt(FOLLOW, msg);
            }
            else{
                cout << "[!] ERROR~ Command not valid" << endl;
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
            std::cerr << "[!] ERROR~ Error receiving data" << std::endl;
        }
        else{
            // Print received data
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            Packet pkt = Packet::deserialize(buffer);

            if(pkt.name == "SERVER")
                cout << "\033[1;34m[!] " << pkt.name << "~ " << pkt._payload << "\033[0m" << endl;
            else
                cout << "[!] " << pkt.name << "~ " << pkt._payload << endl;
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

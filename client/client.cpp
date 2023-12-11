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

    return 0;
}

void Client::login(){
    string msg("Request for login");
    Client::send(LOGIN, msg);
}

void Client::follow(string username){
    Client::send(FOLLOW, username);
}

void Client::send(uint16_t code, string payload){
    Packet packet(code, 0, payload.length(), time(NULL), this->c_info.name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bytesSent == -1) {
        std::cerr << "[!] ERROR~ Error sending data to server" << std::endl;
    }
}

void Client::get_input(){
    while (true){
        char input[BUFFER_SIZE];
        cout << "[#] " << this->c_info.name << "~ ";
        fgets(input, BUFFER_SIZE, stdin);
        string msg = input;
        if(msg.size() > MSG_SIZE){
            cerr << "[!] ERROR~ Message must not be longer than 128 characters" << endl;
        }
        else{
            stringstream tokenizer(msg);
            string code;

            getline(tokenizer, code, ' ');
            
            if(code == "SEND"){
                getline(tokenizer, msg);
                send(SEND, msg);
            }
            else if(code == "FOLLOW"){
                getline(tokenizer, msg);
                send(FOLLOW, msg);
            }
            else{
                cout << "[!] ERROR~ Command not valid" << endl;
            }
        }
    }
}

void Client::listen(){
    if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "[!] ERROR~ Error binding socket" << std::endl;
		close(udpSocket);
        exit(-1);
	}

    while (true){
        // Receive data
        char buffer[1024];
        socklen_t serverAddressLenght = sizeof(serverAddress);
        ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serverAddress, &serverAddressLenght);

        if (bytesRead == -1) {
            std::cerr << "[!] ERROR~ Error receiving data" << std::endl;
        }
        else{
            // Print received data
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            Packet pkt = Packet::deserialize(buffer);

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
    listenThread.join();
}

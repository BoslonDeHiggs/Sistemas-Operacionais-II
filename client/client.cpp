#include "client.hpp"
#include "../packet/packet.hpp"
#include <netdb.h>

#define MULTICAST_ADDR "239.255.255.200"
#define MULTICAST_PORT 8888

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

void Client::init(){
    frontend.init_multicast();
    frontend.call_listenMulticastThread();
    frontend.call_receiveFromServerThread();
    frontend.call_sendToServerThread();
    call_listenThread();
    call_sendThread();
}

Client::~Client() {
    globalClientPointer = nullptr;
}

void Client::login(){
    string login = "Request for login";

    Packet packet(LOGIN, 0, login.length(), time(NULL), this->c_info.name, login);

    unique_lock<mutex> lock_queue_send(frontend.mtx_queue_send);
        frontend.pkt_queue_send.push(packet);
    frontend.cv_queue_send.notify_one();
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
                Packet packet(SEND, 0, msg.length(), timestamp, this->c_info.name, msg);

                unique_lock<mutex> lock_queue_send(frontend.mtx_queue_send);
                    frontend.pkt_queue_send.push(packet);
                frontend.cv_queue_send.notify_one();

                print_send_msg(timestamp, this->c_info.name, code, msg);
            }
            else if(code == "FOLLOW"){
                Packet packet(FOLLOW, 0, msg.length(), timestamp, this->c_info.name, msg);

                unique_lock<mutex> lock_queue_send(frontend.mtx_queue_send);
                    frontend.pkt_queue_send.push(packet);
                frontend.cv_queue_send.notify_one();

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
        unique_lock<mutex> lock_queue_receive(frontend.mtx_queue_receive);
			frontend.cv_queue_receive.wait(lock_queue_receive, [this]() { return !frontend.pkt_queue_receive.empty(); });
		Packet pkt = frontend.pkt_queue_receive.front();
		frontend.pkt_queue_receive.pop();

        print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
    }
}

void Client::call_listenThread(){
    thread listenThread(&Client::listen, this);
    listenThread.detach();
}

void Client::call_sendThread(){
    thread sendThread(&Client::get_input, this);
    sendThread.join();
}

void Client::sendExit(){
    Packet packet(EXIT, 0, 0, time(NULL), this->c_info.name, "Terminating session");

    unique_lock<mutex> lock_queue_send(frontend.mtx_queue_send);
        frontend.pkt_queue_send.push(packet);
    frontend.cv_queue_send.notify_one();
}

void Client::signalHandler(int signal) {    //Teste inicial de encerrar sessao
    print_client_msg("Terminating session (Captured ctrl+C)");
    if (globalClientPointer != nullptr) {
        globalClientPointer->sendExit();
    }

    exit(signal);
}
#include "server.hpp"
#include "../packet/packet.hpp"

using namespace std;

Server::Server(uint16_t port){}

int Server::open_udp_connection(uint16_t port){
	// Open UDP socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) {
		std::cerr << "[!] ERROR~ Error creating socket" << std::endl;
		return -100;
	}
	
	// Set up the server address struct
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Use any available network interface
	serverAddress.sin_port = htons(port);
	bzero(&(serverAddress.sin_zero), 8);

	// Bind the socket to the address and port
	if (bind(udpSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "[!] ERROR~ Error binding socket" << std::endl;
		close(udpSocket);
		return -200;
	}

	return 0;
}

void Server::init_database(){
	int code = this->database.open();
	if(code == -1){
		cerr << "[!] ERROR~ Error while opening file" << endl;
	}
	this->database.read();
}

void Server::listen(){
	while (true){
		// Receive data
		char buffer[1024];
		sockaddr_in clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, &clientAddressLength);

		if (bytesRead == -1) {
			std::cerr << "[!] ERROR~ Error receiving data" << std::endl;
		}
		else{
			// Print received data
			buffer[bytesRead] = '\0'; // Null-terminate the received data

			Packet pkt = Packet::deserialize(buffer);

			pkt_addr packet_address(pkt, clientAddress);

			unique_lock<mutex> lock(mtx);
				pkts_queue.push(packet_address);
			cv.notify_one();
		}
	}
}

void Server::send(sockaddr_in clientAddress, string clientName, string payload){
    Packet packet(0, 0, payload.length(), time(NULL), clientName, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
    if (bytesSent == -1) {
        std::cerr << "[!] SERVER~ Error sending data to client" << std::endl;
    }
}

void Server::process(){
	while(true){
		unique_lock<mutex> lock(mtx);
			cv.wait(lock, [this]() { return !pkts_queue.empty(); });
			pkt_addr packet_address = pkts_queue.front();
			pkts_queue.pop();

		Packet pkt = packet_address.pkt;
		sockaddr_in clientAddress = packet_address.addr;

		
		if (pkt.type == LOGIN) {
			cout << "[!] SERVER~ Request for login from " << pkt.name << endl;
			bool in_database = database.contains(pkt.name);
			if (!in_database) {
				cout << "[!] SERVER~ User doesn't have an account" << endl;
				cout << "[!] SERVER~ Creating account for " << pkt.name << endl;
				database.sign_up(pkt.name, clientAddress);
				send(clientAddress, "SERVER", "Account created successfully");
				database.write();
			} else {
				auto it = database.addressMap.find(pkt.name);
				if (it == database.addressMap.end() || it->second.size() < 2) {
					cout << "[!] SERVER~ " << pkt.name << " logging in" << endl;
					database.login(pkt.name, clientAddress);
					send(clientAddress, "SERVER", "Login successful");
					cout << "Usuario: " << pkt.name << " logando, verificando mensagens pendentes." << endl;
					sendPendingMessages(pkt.name, clientAddress);
				} else {
					send(clientAddress, "SERVER", "There are two other sessions already active");
				}
			}
		}
		else{
			bool logged = database.is_logged_in_addr(pkt.name, clientAddress);
			if(logged){
				if(pkt.type == SEND){
					cout << "[!] " << pkt.name << "~ " << pkt._payload << endl;
					vector<string> followers = database.get_followers(pkt.name);
					for(const string& follower : followers){
						if(database.is_logged_in(follower)){
							map<string, vector<sockaddr_in>>::iterator it;
							it = database.addressMap.find(follower);
							for(sockaddr_in address : it->second)
								send(address, pkt.name, pkt._payload);
						}
						else{
							cout << "Armazenando mensagem para ser enviada mais tarde para: " << follower << endl;
							database.storeMessageForOfflineUser(follower, pkt);
							if (!database.messageQueue[follower].empty()){
								cout << "Mensagem armazenada com sucesso" << endl;
							}
						}
					}
				}
				else if(pkt.type == FOLLOW){
					bool in_database = database.contains(pkt._payload);
					if(in_database){
						if(pkt._payload != pkt.name){
							bool successfull = database.add_follower(pkt._payload, pkt.name);
							if(successfull){
								database.write();
								cout << "[!] SERVER~ " << pkt.name << " started following " << pkt._payload << endl;
								send(clientAddress, "SERVER", "You started following " + pkt._payload);
							}
							else send(clientAddress, "SERVER", "You already follow " + pkt._payload);
						}
						else{
							send(clientAddress, "SERVER", "Can't follow self");
						}
					}
					else{
						cout << "[!] SERVER~ Something went wrong" << endl;
						send(clientAddress, "SERVER", "Something went wrong");
					}
				}
 				else if(pkt.type == EXIT){
					bool in_database = database.contains(pkt.name);
					if(in_database){
						database.exit(pkt.name, clientAddress);
						cout << "[!] SERVER~ " << pkt.name << " exited" << endl;
					}
					else{
						cout << "[!] SERVER~ Something went wrong while logging out" << endl;
					}
				}
			}
		}
	}
}

void Server::call_listenThread(){
	thread listenThread(&Server::listen, this);
	listenThread.join();
}

void Server::call_processThread(){
	thread processThread(&Server::process, this);
	processThread.detach();
}

void Server::sendPendingMessages(const string& username, const sockaddr_in& clientAddress) {
    auto& queue = database.messageQueue[username];
    while (!queue.empty()) {
        Packet messagePkt = queue.front();
        send(clientAddress, messagePkt.name, messagePkt._payload);
        queue.pop();
    }
}
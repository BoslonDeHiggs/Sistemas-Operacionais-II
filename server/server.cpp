#include "server.hpp"
#include "../packet/packet.hpp"

using namespace std;

Server::Server(uint16_t port){}

int Server::open_udp_connection(uint16_t port){
	// Open UDP socket
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1) {
		print_error_msg("Error creating socket");
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
		print_error_msg("Error binding socket");
		close(udpSocket);
		return -200;
	}

	return 0;
}

void Server::init_database(){
	int code = this->database.open();
	if(code == -1){
		print_error_msg("Error while opening file");
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
			print_error_msg("Error receiving data");
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

void Server::send(sockaddr_in clientAddress, time_t timestamp, string clientName, string payload){
    Packet packet(0, 0, payload.length(), timestamp, clientName, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
    if (bytesSent == -1) {
        print_error_msg("Error sending data to client");
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
			print_server_ntf(pkt.timestamp, "Request for login from", pkt.name);
			bool in_database = database.contains(pkt.name);
			if (!in_database) {
				print_server_ntf(time(NULL), "User doesn't have an account", "");
				print_server_ntf(time(NULL), "Creating account for", pkt.name);
				database.sign_up(pkt.name, clientAddress);
				send(clientAddress, time(NULL), "SERVER", "Account created successfully");
				database.write();
			} else {
				auto it = database.addressMap.find(pkt.name);
				if (it == database.addressMap.end() || it->second.size() < 2) {
					print_server_ntf(time(NULL), "Login from", pkt.name);
					database.login(pkt.name, clientAddress);
					send(clientAddress, time(NULL), "SERVER", "Login successful");
					print_server_ntf(time(NULL), "Verifying pending messages for", pkt.name);
					sendPendingMessages(pkt.name, clientAddress);
				} else {
					print_server_ntf(time(NULL), "There are two other sessions already active from", pkt.name);
					send(clientAddress, time(NULL), "SERVER", "There are two other sessions already active");
				}
			}
		}
		else{
			bool logged = database.is_logged_in_addr(pkt.name, clientAddress);
			if(logged){
				if(pkt.type == SEND){
					print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
					vector<string> followers = database.get_followers(pkt.name);
					for(const string& follower : followers){
						if(database.is_logged_in(follower)){
							map<string, vector<sockaddr_in>>::iterator it;
							it = database.addressMap.find(follower);
							for(sockaddr_in address : it->second)
								send(address, pkt.timestamp, pkt.name, pkt._payload);
						}
						else{
							print_server_ntf(time(NULL), "Storing messages in message queue for", follower);
							database.storeMessageForOfflineUser(follower, pkt);
							if (!database.messageQueue[follower].empty()){
								print_server_ntf(time(NULL), "Message stored successfully", "");
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
								print_server_follow_ntf(time(NULL), "started following", pkt.name, pkt._payload);
								send(clientAddress, time(NULL), "SERVER", "You started following " + pkt._payload);
							}
							else send(clientAddress, time(NULL), "SERVER", "You already follow " + pkt._payload);
						}
						else{
							send(clientAddress, time(NULL), "SERVER", "Can't follow self");
						}
					}
					else{
						print_server_ntf(time(NULL), "Something went wrong", "");
						send(clientAddress, time(NULL), "SERVER", "Something went wrong");
					}
				}
 				else if(pkt.type == EXIT){
					bool in_database = database.contains(pkt.name);
					if(in_database){
						database.exit(pkt.name, clientAddress);
						print_server_ntf(time(NULL), "Log out from", pkt.name);
					}
					else{
						print_server_ntf(time(NULL), "Something went wrong while logging out", "");
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
        send(clientAddress, messagePkt.timestamp, messagePkt.name, messagePkt._payload);
        queue.pop();
    }
}
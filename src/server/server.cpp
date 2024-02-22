#include "server.hpp"
#include "../packet/packet.hpp"

#define PORT 4000
#define BROADCAST_PORT 16384

using namespace std;

Server::Server(uint16_t port){
	id = time(NULL);
	leader = false;

	init_database();
	open_udp_connection(PORT);
	create_broadcast_socket();
	send_broadcast_pkt(NEW_SERVER, time(NULL), "SERVER"+to_string(id), to_string(id));
	call_listenBroadcastThread();
	call_listenThread();
	call_heartbeatThread();
	call_processBroadcastThread();
	call_processThread();
	
//ALGORITMO DE ELEICAO
//	std::thread heartbeatCheckThread(&Server::checkHeartbeatTimeout, this);
//	heartbeatCheckThread.detach(); // Executa em background
}

string Server::get_own_address(int sockfd){
    sockaddr_in sockname;
    socklen_t socklen = sizeof(sockname);

    // Retrieve the socket address information
    if (getsockname(sockfd, reinterpret_cast<sockaddr*>(&sockname), &socklen) == -1) {
        return "";
    }

    // Convert the IP address to a string
    char buffer[INET_ADDRSTRLEN];
    const char* p = inet_ntop(AF_INET, &sockname.sin_addr, buffer, sizeof(buffer));
    if (p == nullptr) {
        return "";
    }

    // Convert port to string
    uint16_t port = ntohs(sockname.sin_port);

    // Return the IP address and port as a string
    return std::string(p) + ":" + std::to_string(port);
}


void Server::init_database(){
	int code = this->database.open();
	if(code == -1){
		print_error_msg("Error while opening file");
	}
	this->database.read();
}

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

int Server::create_broadcast_socket() {
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

	// Bind broadcast socket
    if (bind(broadcastSocket, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == -1) {
        print_error_msg("Error binding broadcast socket");
        close(broadcastSocket);
        return -300;
    }

    return 0;
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

void Server::listen_broadcast() {
    while (true) {
		char buffer[1024];
		sockaddr_in clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		ssize_t bytesRead = recvfrom(broadcastSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, &clientAddressLength);

        if (bytesRead == -1) {
            print_error_msg("Error receiving broadcast message");
        } else {
            buffer[bytesRead] = '\0'; // Null-terminate the received data

			Packet pkt = Packet::deserialize(buffer);

			pkt_addr packet_address(pkt, clientAddress);

			unique_lock<mutex> lock(mtx_broadcast);
				pkts_queue_broadcast.push(packet_address);
			cv_broadcast.notify_one();
        }
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
			handleLogin(pkt,clientAddress);
		}
		else{
			bool logged = database.is_logged_in_addr(pkt.name, clientAddress);
			if(logged){
				if(pkt.type == SEND){
					handleSend(pkt, clientAddress);
				}
				else if(pkt.type == FOLLOW){
					handleFollow(pkt,clientAddress);
				}
 				else if(pkt.type == EXIT){
					handleExit(pkt,clientAddress);
				}
			}
		}
	}
}

void Server::process_broadcast(){
	while(true){
		unique_lock<mutex> lock(mtx_broadcast);
		cv_broadcast.wait(lock, [this]() { return !pkts_queue_broadcast.empty(); });
		pkt_addr packet_address = pkts_queue_broadcast.front();
		pkts_queue_broadcast.pop();

		Packet pkt = packet_address.pkt;
		sockaddr_in address = packet_address.addr;

		if(pkt.type == DISCOV_MSG){
			if(leader){
				string payload = "You are connected to the main server";
				send_pkt(DISCOV_MSG, address, time(NULL), "SERVER", payload);
			}
		}
		if(pkt.type == HEARTBEAT){
			//ALGORITMO DE ELEICAO
			//lastHeartbeat = time(NULL);
			print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
		}
		if(pkt.type == NEW_SERVER){
			std::ifstream infile("database/database.txt");
            std::string database_content;
            std::string line;
			string token = "<<newline>>";
            while (std::getline(infile, line)) {
                database_content += line + token;
            }
			if (!database_content.empty()) {
        		database_content.erase(database_content.length() - token.length(), token.length());
    		}

			address.sin_port = PORT;

            infile.close();
			cout << database_content << endl;
			send_broadcast_pkt(LEADER_CHECK, time(NULL), "SERVER", database_content);
		}
		if(pkt.type == DATABASE){
			//cout << "Cade o Banco de dados?" << endl;
			print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
			//cout << "Aqui em cima" << endl;
			string received_content = pkt._payload;
    		size_t pos = 0;
			string token = "<<newline>>";
    		while ((pos = received_content.find(token, pos)) != string::npos) {
        		received_content.replace(pos, token.length(), "\n");
        		pos += 1; // Movendo alem do '\n' substituido para evitar loop infinito
    		}
			std::ofstream outfile("new_database.txt"); // Abre o arquivo "new_database.txt" para escrita
			if (outfile.is_open()) {
				outfile << received_content; // Escreve o conteúdo no arquivo
				outfile.close(); // Fecha o arquivo
			} else {
				std::cerr << "Não foi possível abrir o arquivo para escrita." << std::endl;
			}
		}
		if(pkt.type == LEADER_CHECK){
			print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);

			if (pkt.timestamp < id){ //Se um servidor de timestamp menor que o meu me responder, atualizo meu banco de dados para o dele e fico em stand-by
				cout << "Eu nao sou o lider" << endl;
				//send_broadcast_pkt(DATABASE, time(NULL), "SERVER", pkt._payload);
			}
			else{
				cout << "Eu sou o lider" << endl;
				send_broadcast_pkt(DATABASE, time(NULL), "SERVER", pkt._payload); // Essa linha nao esta correta aqui, apenas para fins de teste e debug
			}
		}
	}
}

void Server::heartbeat(){
	string payload = "Heartbeat";
	while(true){
		sleep(3);
		send_broadcast_pkt(HEARTBEAT, time(NULL), "SERVER " + to_string(id), payload);
	}
}

void Server::send_pkt(uint16_t type, sockaddr_in clientAddress, time_t timestamp, string name, string payload){
    Packet packet(type, 0, payload.length(), timestamp, name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
    if (bytesSent == -1) {
        print_error_msg("Error sending data to client");
    }
}

void Server::send_broadcast_pkt(uint16_t type, time_t timestamp, string name, string payload){
    Packet packet(type, 0, payload.length(), timestamp, name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(broadcastSocket, message, strlen(message), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    if (bytesSent == -1) {
        print_error_msg("Error sending data to client");
    }
}

void Server::handleLogin(const Packet& pkt, const sockaddr_in& clientAddress) {
    print_server_ntf(pkt.timestamp, "Request for login from", pkt.name);
    
    bool in_database = database.contains(pkt.name);

    if (!in_database) {
        handleNewAccount(pkt, clientAddress);
    } else {
        handleExistingAccount(pkt, clientAddress);
    }
}

void Server::handleNewAccount(const Packet& pkt, const sockaddr_in& clientAddress) {
    print_server_ntf(time(NULL), "User doesn't have an account", "");
    print_server_ntf(time(NULL), "Creating account for", pkt.name);
    database.sign_up(pkt.name, clientAddress);
    send_pkt(0, clientAddress, time(NULL), "SERVER", "Account created successfully");
    database.write();
}

void Server::handleExistingAccount(const Packet& pkt, const sockaddr_in& clientAddress) {
    auto it = database.addressMap.find(pkt.name);
    if (it == database.addressMap.end() || it->second.size() < 2) {
        print_server_ntf(time(NULL), "Login from", pkt.name);
        database.login(pkt.name, clientAddress);
        send_pkt(0, clientAddress, time(NULL), "SERVER", "Login successful");
        print_server_ntf(time(NULL), "Verifying pending messages for", pkt.name);
        sendPendingMessages(pkt.name, clientAddress);
    } else {
        print_server_ntf(time(NULL), "There are two other sessions already active from", pkt.name);
        send_pkt(0, clientAddress, time(NULL), "SERVER", "There are two other sessions already active");
    }
}

void Server::handleSend(const Packet& pkt, const sockaddr_in& clientAddress) {
	print_rcv_msg(pkt.timestamp, pkt.name, pkt._payload);
	vector<string> followers = database.get_followers(pkt.name);
	for(const string& follower : followers){
		if(database.is_logged_in(follower)){
			map<string, vector<sockaddr_in>>::iterator it;
			it = database.addressMap.find(follower);
			for(sockaddr_in address : it->second)
				send_pkt(0, address, pkt.timestamp, pkt.name, pkt._payload);
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

void Server::handleFollow(const Packet& pkt, const sockaddr_in& clientAddress) {
    bool in_database = database.contains(pkt._payload);
    
    if (in_database) {
        if (pkt._payload != pkt.name) {
            bool successful = database.add_follower(pkt._payload, pkt.name);
            if (successful) {
                database.write();
                print_server_follow_ntf(time(NULL), "started following", pkt.name, pkt._payload);
                send_pkt(0, clientAddress, time(NULL), "SERVER", "You started following " + pkt._payload);
            } else {
                send_pkt(0, clientAddress, time(NULL), "SERVER", "You already follow " + pkt._payload);
            }
        } else {
            send_pkt(0, clientAddress, time(NULL), "SERVER", "Can't follow self");
        }
    } else {
        print_server_ntf(time(NULL), "Something went wrong", "");
        send_pkt(0, clientAddress, time(NULL), "SERVER", "Something went wrong");
    }
}

void Server::handleExit(const Packet& pkt, const sockaddr_in& clientAddress) {
    bool in_database = database.contains(pkt.name);
    
    if (in_database) {
        database.exit(pkt.name, clientAddress);
        print_server_ntf(time(NULL), "Log out from", pkt.name);
    } else {
        print_server_ntf(time(NULL), "Something went wrong while logging out", "");
    }
}

void Server::call_listenThread(){
	thread listenThread(&Server::listen, this);
	listenThread.detach();
}

void Server::call_listenBroadcastThread(){
	thread listenBroadcastThread(&Server::listen_broadcast, this);
	listenBroadcastThread.detach();
}

void Server::call_processThread(){
	thread processThread(&Server::process, this);
	processThread.join();
}

void Server::call_processBroadcastThread(){
	thread processBroadcastThread(&Server::process_broadcast, this);
	processBroadcastThread.detach();
}

void Server::call_heartbeatThread(){
	thread heartbeatThread(&Server::heartbeat, this);
	heartbeatThread.detach();
}

void Server::sendPendingMessages(const string& username, const sockaddr_in& clientAddress) {
    auto& queue = database.messageQueue[username];
    while (!queue.empty()) {
        Packet messagePkt = queue.front();
        send_pkt(0, clientAddress, messagePkt.timestamp, messagePkt.name, messagePkt._payload);
        queue.pop();
    }
}

int Server::get_socket(){
	return this->udpSocket;
}

//ALGORITMO DE ELEICAO
//void Server::checkHeartbeatTimeout() {
//    while(true) {
//        sleep(1); // Verifica o timeout a cada segundo
//
//        if(lastHeartbeat > 0 && (time(NULL) - lastHeartbeat) > HEARTBEAT_TIMEOUT) { //Se a diferença de tempo entre a ultima heartbeat e o tempo
//            std::cout << "Servidor primário inativo. Iniciando eleição de novo líder..." << std::endl; //atual for maior que o limite, inicia eleicao
//            // Implemente a lógica de eleição de novo líder ou outra ação aqui
//            lastHeartbeat = 0; // Reset para evitar múltiplas detecções
//        }
//    }
//}
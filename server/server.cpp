
#include "server.hpp"
#include "../packet/packet.hpp"

using namespace std;

Server::Server(uint16_t port){
	//Todo o codigo destes colchetes faz parte do teste para multicast
	multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr("239.255.255.250");
    multicastAddr.sin_port = htons(port);
	setup_multicast(port);
	std::thread(&Server::listen_multicast, this).detach();
	send_multicast_initial_message();
	//send_multicast("Servidor iniciado e juntando-se ao grupo multicast");
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

void Server::init_database(){
	int code = this->database.open();
	if(code == -1){
		print_error_msg("Error while opening file");
	}
	this->database.read();
}

void Server::listen(){
	while (true){
		// ie data
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

			unique_lock<mutex> lock_listen_process(mtx_listen_process);
				pkts_queue_listen_process.push(packet_address);
			cv_listen_process.notify_one();
		}
	}
}

void Server::process(){
	while(true){
		unique_lock<mutex> lock_listen_process(mtx_listen_process);
			cv_listen_process.wait(lock_listen_process, [this]() { return !pkts_queue_listen_process.empty(); });
		pkt_addr packet_address = pkts_queue_listen_process.front();
		pkts_queue_listen_process.pop();

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

void Server::send(){
	while(true){
		unique_lock<mutex> lock_process_send(mtx_process_send);
			cv_process_send.wait(lock_process_send, [this]() { return !pkts_queue_process_send.empty(); });
		pkt_addr packet_address = pkts_queue_process_send.front();
		pkts_queue_process_send.pop();

		Packet pkt = packet_address.pkt;
		sockaddr_in clientAddress = packet_address.addr;

		send_pkt(clientAddress, pkt.timestamp, pkt.name, pkt._payload);
	}
}

void Server::send_pkt(sockaddr_in clientAddress, time_t timestamp, string clientName, string payload){
    Packet packet(0, 0, payload.length(), timestamp, clientName, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(udpSocket, message, strlen(message), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
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

	Packet packet(0, 0, 0, time(NULL), "SERVER", "Account created successfully");
	pushSendQueue(packet, clientAddress);

    database.write();
}

void Server::handleExistingAccount(const Packet& pkt, const sockaddr_in& clientAddress) {
    auto it = database.addressMap.find(pkt.name);
    if (it == database.addressMap.end() || it->second.size() < 2) {
        print_server_ntf(time(NULL), "Login from", pkt.name);
        database.login(pkt.name, clientAddress);

		Packet packet(0, 0, 0, time(NULL), "SERVER", "Login successful");
		pushSendQueue(packet, clientAddress);

        print_server_ntf(time(NULL), "Verifying pending messages for", pkt.name);
        sendPendingMessages(pkt.name, clientAddress);
    } else {
        print_server_ntf(time(NULL), "There are two other sessions already active from", pkt.name);

		Packet packet(0, 0, 0, time(NULL), "SERVER", "There are two other sessions already active");
		pushSendQueue(packet, clientAddress);
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
			{
				Packet packet(0, 0, 0, pkt.timestamp, pkt.name, pkt._payload);
				pushSendQueue(packet, address);
			}
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

				Packet packet(0, 0, 0, time(NULL), "SERVER", "You started following " + pkt._payload);
				pushSendQueue(packet, clientAddress);
            } else {
				Packet packet(0, 0, 0, time(NULL), "SERVER", "You already follow " + pkt._payload);
				pushSendQueue(packet, clientAddress);
            }
        } else {
			Packet packet(0, 0, 0, time(NULL), "SERVER", "Can't follow self");
			pushSendQueue(packet, clientAddress);
        }
    } else {
        print_server_ntf(time(NULL), "Something went wrong", "");

		Packet packet(0, 0, 0, time(NULL), "SERVER", "Something went wrong");
		pushSendQueue(packet, clientAddress);
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

void Server::sendPendingMessages(const string& username, const sockaddr_in& clientAddress) {
    auto& queue = database.messageQueue[username];
    while (!queue.empty()) {
        Packet messagePkt = queue.front();

		Packet packet(0, 0, 0, messagePkt.timestamp, messagePkt.name, messagePkt._payload);
		pushSendQueue(packet, clientAddress);

        queue.pop();
    }
}

void Server::pushSendQueue(const Packet& pkt, const sockaddr_in& clientAddress){
	pkt_addr packet_address(pkt, clientAddress);

	unique_lock<mutex> lock_process_send(mtx_process_send);
		pkts_queue_process_send.push(packet_address);
	cv_process_send.notify_one();
}

void Server::call_listenThread(){
	thread listenThread(&Server::listen, this);
	listenThread.detach();
}

void Server::call_processThread(){
	thread processThread(&Server::process, this);
	processThread.detach();
}

void Server::call_sendThread(){
	thread sendThread(&Server::send, this);
	sendThread.join();
}

int Server::get_socket(){
	return this->udpSocket;
}

void Server::setup_multicast(uint16_t port) {
    // Criacao do socket multicast
    multicastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicastSocket < 0) {
        perror("Multicast socket creation failed");
        exit(EXIT_FAILURE);
    }

	int reuse = 1;
    if (setsockopt(multicastSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR failed");
        close(multicastSocket);
        exit(EXIT_FAILURE);
    }

    // Definindo o endereco e porta do grupo multicast
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr("239.255.255.250"); // Exemplo de endereco multicast
    multicastAddr.sin_port = htons(port);

    // Associacao do socket ao endereco multicast
    if (bind(multicastSocket, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr)) < 0) {
        perror("Multicast socket bind failed");
        close(multicastSocket);
        exit(EXIT_FAILURE);
    }

    // Juncao ao grupo multicast
    struct ip_mreq mreq;
    mreq.imr_multiaddr = multicastAddr.sin_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Multicast group join failed");
        close(multicastSocket);
        exit(EXIT_FAILURE);
    }

    // O socket esta agora configurado para ouvir mensagens multicast
}

void Server::send_multicast(const std::string& message) {
    // Envio da mensagem para o grupo multicast
    int bytesSent = sendto(multicastSocket, message.c_str(), message.length(), 0,
                           (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
    if (bytesSent < 0) {
        perror("Sending multicast message failed");
    }
}

void Server::send_multicast_initial_message() {
    auto timestamp = time(NULL); // Obtem o timestamp atual

	//char serverIP[INET_ADDRSTRLEN];
    //inet_ntop(AF_INET, &(serverAddress.sin_addr), serverIP, INET_ADDRSTRLEN);

	//auto serverPort = ntohs(serverAddress.sin_port);
    
	//std::string message = "Servidor iniciado; IP: " + std::string(serverIP) + "; Porta: " + std::to_string(serverPort) + "; ID: " + std::to_string(timestamp);
    //std::string message = "Servidor iniciado - ID: " + std::to_string(timestamp);
	std::string port = std::to_string(ntohs(serverAddress.sin_port)); // Porta em que o servidor esta ouvindo

    std::string message = "Servidor iniciado - Porta: " + port + "; ID: " + std::to_string(timestamp);
    send_multicast(message);
}

void Server::listen_multicast() {
    // Buffer para receber mensagens multicast
    char buffer[1024];
    while (true) {
        // Escuta por mensagens multicast
        socklen_t addrlen = sizeof(multicastAddr);
        int bytesRead = recvfrom(multicastSocket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr*)&multicastAddr, &addrlen);
        if (bytesRead < 0) {
            perror("Receiving multicast message failed");
            continue;
        }

        // Terminacao nula dos dados recebidos e processamento da mensagem
        buffer[bytesRead] = '\0';
        std::string receivedMessage(buffer);
        std::cout << "Received multicast message - IP: " << inet_ntoa(multicastAddr.sin_addr) << " - PORT: " << ntohs(multicastAddr.sin_port) << " " << receivedMessage << std::endl;

        // Aqui voce pode analisar a mensagem recebida e responder se necessario
        // Este e o lugar para implementar a logica de resposta ao multicast recebido
    }
}
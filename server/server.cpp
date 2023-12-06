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

			if(pkt.type == SEND){
				cout << "[!] <" << pkt.name << ">~ " << pkt._payload << endl;
			}
			else if(pkt.type == LOGIN){
				cout << "[!] SERVER~ " << pkt._payload << endl;
			}
		}
	}
}

void appendUserNameToFile(const string& content, const string& filename) {
    // Open the file in append mode
    ofstream outFile(filename, ios::app);

    if (outFile.is_open()) {
        // Append the content to the file
        outFile << content << endl;

        // Close the file
        outFile.close();

        std::cout << "User Name appended to file successfully." << endl;
    } else {
        std::cerr << "Unable to open the file for appending." << endl;
    }
}

bool isUserNameInFile(const string& name, const string& filename) {
    ifstream inFile(filename);

    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            if (line == name) {
                inFile.close();
                return true; // Name is already in the file
            }
        }

        inFile.close();
        return false; // Name is not in the file
    } else {
        cerr << "Unable to open the file for reading." << endl;
        return false; // Error reading the file
    }
}

void processUserName(const string& name, const string& filename) {
    if (isUserNameInFile(name, filename)) {
        cout << "User Name is already in the file." << endl;
    } else {
        // Append the name to the file
        appendUserNameToFile(name, filename);
    }
}

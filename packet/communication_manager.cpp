#include "communication_manager.hpp"
#include "../format/format.hpp"

void CommunicationManager::send_packet(int socket, sockaddr_in dst_address, int type, time_t timestamp, string src_name, string payload){
    Packet packet(type, 0, payload.length(), timestamp, src_name, payload);

    string aux = packet.serialize();

    const char* message = aux.c_str();
    ssize_t bytesSent = sendto(socket, message, strlen(message), 0, (struct sockaddr*)&dst_address, sizeof(dst_address));
    if (bytesSent == -1) {
        print_error_msg("Error sending data");
    }
}
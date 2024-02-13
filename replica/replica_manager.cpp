#include "replica_manager.hpp"
#include "../packet/communication_manager.hpp"
#include <string>
#include <netdb.h>
#include <iostream>
using namespace std;

sockaddr_in get_addr_from_ip_and_port(const char* ip, uint16_t port){
    struct hostent *host = gethostbyname(ip);
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *((struct in_addr *)host->h_addr);
    addr.sin_port = htons(port);
    bzero(&(addr.sin_zero), 8);
    return addr;
}

ReplicaManager::ReplicaManager(uint16_t replica_port, const char* leader_ip, uint16_t leader_port) : Server(replica_port){

    // Set up the leader address struct
    this->leader_addr = get_addr_from_ip_and_port(leader_ip, leader_port);
    this->id = getpid();
    //Send message to leader -> leader will add this replica to his replicas map
    //Leader will have to send the updated replicas map to this replica and his id
}

void ReplicaManager::start_election(){
	/*
	1. send election message to all replicas with higher id than this replica
	2. wait 'timeout' seconds listening on replicas, 
	if it does not get an answer then send COORDINATOR message to all replicas (won election)
	3. if it receives an answer then wait until rececives a COORDINATOR message 
	4. if it does not receive an coordinator message in time then restart election
    5. if receives coordinator message then update all replicas 
    OBS, another thread will handle the receiving of answer and coordinator message and update got_answer and leader_id accordingly.
        it can be an extension of Server::process
	*/

	int previous_leader = this->leader_id;
    this->election_started = true;
    replicas.erase(previous_leader);
    this->leader_id = ERROR_ID;
    string empty = '\0';

    string replica_name = "SERVER" + to_string(this->id);
    // While new leader is not elected
    while (this->leader_id == ERROR_ID)
    {
        this->got_answer = false;

        // Send messages to every higher ID replica
        for (auto i = replicas.begin(); i != replicas.end(); ++i){
            if (i->first > this->id)
                CommunicationManager::send_packet(this->get_socket(),i->second, ELECTION_START, time(NULL), replica_name, empty);
        }
        
        // Wait for answers
        print_server_ntf(time(NULL),"Sent election messages, waiting...",replica_name);

        sleep(ELECTION_TIMEOUT);

        // After waking up, check for answers
        if (!this->got_answer)
        {
           print_server_ntf(time(NULL),"Got no answers, I am the new coordinator ", replica_name);

            // If no answers arrived, this is the new coordinator
            this->become_leader();
        }
        else
        {
            // Wait for coordinator
            print_server_ntf(time(NULL), "Got an answer, waiting again...", replica_name);

            sleep(ELECTION_TIMEOUT);

            if (this->leader_id == ERROR_ID)
            {
                print_server_ntf(time(NULL), "Still no answers, restarting election...", replica_name);
            }
        }
    }
    string msg = "Election finished, new leader is " + to_string(this->leader_id);
    print_server_ntf(time(NULL),msg, replica_name);

    // End election
    this->election_started = false;
}

void ReplicaManager::become_leader(){

    this->leader_id = this->id;
    print_server_ntf(time(NULL), "Virando o lider id:", to_string(this->id));

    string replica_name = to_string(this->id);
    string empty;
    //Enviar mensagem para outras replicas
    for (auto i = replicas.begin(); i != replicas.end(); ++i)
    if(i->first != this->id)
    {
            CommunicationManager::send_packet(this->get_socket(),i->second, ELECTION_COORDINATOR, time(NULL), replica_name, empty);
    }
    
    print_server_ntf(time(NULL), "Lider pronto: ", to_string(this->id));
}

void ReplicaManager::process_manager(){

}
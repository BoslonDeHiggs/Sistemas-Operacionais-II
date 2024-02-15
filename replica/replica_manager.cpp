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

ReplicaManager::ReplicaManager(uint16_t replica_port, int is_leader) : Server(replica_port){

    // Set up the leader address struct
    this->id = time(NULL);
    if (is_leader)
        this->leader_id = this->id;
    else
        this->leader_id = ERROR_ID;
    this->call_listenThread_multicast();
    this->call_processThread_multicast();   
    ReplicaInfo info = ReplicaInfo(this->id, (uint16_t)this->serverAddress.sin_port, this->serverAddress.sin_addr);
    string message = info.serialize();
    Packet packet = Packet(LINK_REPLICA,0,message.length(),time(NULL),to_string(this->id),message);
    string serialized_packet = packet.serialize();
    send_multicast(serialized_packet);
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
    string empty = "\0";

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
void ReplicaManager::update_replica_on_replica_address(sockaddr_in replica_addr){
    cerr<<"upodate";
    //serialize all info needed for update
    string message_queue = UpdateMessageQueueInfo(this->database.messageQueue).serialize();
    string replicas_map_and_leader = UpdateReplicasInfo(this->leader_id,this->replicas.size(), this->replicas).serialize();
    string followers_map = UpdateFollowersInfo(this->database.followersMap).serialize();
    string address_map = UpdateAddressMapInfo(this->database.addressMap).serialize();
    //create all packets for update
    Packet update_replicas_list = Packet(UPDATE_REPLICAS_LIST, 0 , replicas_map_and_leader.length(), time(NULL), "PRIMARY SERVER", replicas_map_and_leader);
    Packet update_replicas_followers = Packet(UPDATE_REPLICA_FOLLOWERS,0, followers_map.length(), time(NULL), "PRIMARY SERVER", followers_map);
    Packet update_replicas_client_queue = Packet(UPDATE_REPLICA_CLIENT_PACKET_QUEUE, 0, message_queue.length(), time(NULL), "PRIMARY SERVER", message_queue);
    Packet update_replicas_name_to_address_map = Packet(UPDATE_REPLICA_ADDRESS_MAP, 0, address_map.length(), time(NULL), "PRIMARY SERVER", address_map);
    //send packets
    this->pushSendQueue(update_replicas_list, replica_addr);
    this->pushSendQueue(update_replicas_followers,replica_addr);
    this->pushSendQueue(update_replicas_client_queue,replica_addr);
    this->pushSendQueue(update_replicas_name_to_address_map,replica_addr);
}

void ReplicaManager::handle_link_replica(Packet packet){
	//The replica that sent this packet wants to link themselves to this object
	ReplicaInfo info = ReplicaInfo::deserialize(packet._payload);
    //if replica that sent is different from this replica
    if (info.id != this->id){
        cerr<<"qweas";

        in_addr ip;
        ip = info.ip;
        sockaddr_in replica_addr = get_addr_from_ip_and_port(inet_ntoa(ip), info.port);
        int replica_id = info.id;
        this->replicas.insert({replica_id,replica_addr});
        if (this->id == this->leader_id) {
            update_replica_on_replica_address(replica_addr);
        }
    }
}

void ReplicaManager::handle_update_replica(Packet packet){}

void ReplicaManager::handle_election_coordinator(Packet packet){
	ReplicaInfo info = ReplicaInfo::deserialize(packet._payload);
    this->leader_id = info.id;
    //just to be sure do this:
    in_addr ip;
    ip = info.ip;
    sockaddr_in addr = get_addr_from_ip_and_port(inet_ntoa(ip), info.port);
    this->replicas.insert({this->leader_id, addr});
}
void ReplicaManager::process_multicast(){
	while(true){
        unique_lock<mutex> lock_listen_process(mtx_listen_process_multicast);
            cv_listen_process_multicast.wait(lock_listen_process, [this]() { return !pkts_queue_listen_process_multicast.empty(); });
        pkt_addr packet_address = pkts_queue_listen_process_multicast.front();
        pkts_queue_listen_process_multicast.pop();

        Packet pkt = packet_address.pkt;
        //sockaddr_in clientAddress = packet_address.addr;
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
        switch (pkt.type)
        {
        case LINK_REPLICA:
            handle_link_replica(pkt);
            break;
        case UPDATE_REPLICA_CLIENT_PACKET_QUEUE:
        case UPDATE_REPLICA_FOLLOWERS:
        case UPDATE_REPLICA_ADDRESS_MAP:
        case UPDATE_REPLICAS_LIST:
            handle_update_replica(pkt);
            break;
        case ELECTION_COORDINATOR:
            handle_election_coordinator(pkt);
            break;
        default:
            break;
        }
    }
}

void ReplicaManager::call_processThread_multicast(){
	std::thread(&ReplicaManager::process_multicast, this).detach();
}

void ReplicaManager::process(){
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
                    update_replica_on_replica_address(clientAddress)
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
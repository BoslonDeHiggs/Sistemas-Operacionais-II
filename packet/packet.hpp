#pragma once

#include <cstdint>
#include <string>
#include <bits/stdc++.h>
#include <arpa/inet.h>

#define MSG_SIZE 128
#define NUM_PARAM 5
#define NUM_PARAM_REPLICA_INFO 3
#define NUM_PARAM_UPDATE_REPLICAS_INFO 3
#define SEND 100
#define LOGIN 200
#define FOLLOW 300
#define EXIT 400
#define ELECTION_START 500
#define ELECTION_ANSWER 600
#define ELECTION_COORDINATOR 700
#define LINK_REPLICA 900 // will link the new replica to the primary replica (just add to primary map of replicas)
#define UPDATE_REPLICAS_LIST 1000 //
#define UPDATE_REPLICA_CLIENT_PACKET_QUEUE 1001
#define UPDATE_REPLICA_FOLLOWERS 1002
#define UPDATE_REPLICA_ADDRESS_MAP 1003
#define SERVICE_DISCOVERY 1100

using namespace std;

class Packet{
    public:
        uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
        uint16_t seqn; //Número de sequência
        uint16_t length; //Comprimento do payload
        time_t timestamp; // Timestamp do dado
        string name; //Nome do remetente
        string _payload; //Dados da mensagem

        Packet(uint16_t type, uint16_t seqn, uint16_t length, time_t timestamp, string name, string _payload);

        string serialize();

        static Packet deserialize(string serpkt);

    private:
};
typedef struct pkt_addr{
    Packet pkt;
    sockaddr_in addr;

    pkt_addr(Packet packet, sockaddr_in address) : pkt(packet), addr(address){}
}pkt_addr;

class ReplicaInfo{
    public:
        int id; //replica id    
        uint16_t port; //replica port
        in_addr ip; // Timestamp do dado

        ReplicaInfo(int id, uint16_t port, in_addr ip);

        string serialize();

        static ReplicaInfo deserialize(string serpkt);
};

class UpdateReplicasInfo{
    public:
        int leader_id; //replica id    
        int number_of_replicas; 
        map<int,sockaddr_in> replicas; //replicas

        UpdateReplicasInfo(int leader_id, int number_of_replicas, map<int, sockaddr_in> replicas);

        string serialize();

        static UpdateReplicasInfo deserialize(string serpkt);
};

class UpdateFollowersInfo{
    public:
        map<string, vector<string>> followersMap;

        UpdateFollowersInfo(map<string, vector<string>> followersMap);

        string serialize();

        static UpdateFollowersInfo deserialize(string serpkt);
};
class UpdateMessageQueueInfo{
    public:
        map<string, queue<Packet>> client_packet_queue; //packets sent by clients

        UpdateMessageQueueInfo(map<string, queue<Packet>> client_packet_queue);

        string serialize();

        static UpdateMessageQueueInfo deserialize(string serpkt);
};

class UpdateAddressMapInfo{
    public:
        map<string, vector<sockaddr_in>> addressMap;

        UpdateAddressMapInfo(map<string, vector<sockaddr_in>> addressMap);

        string serialize();

        static UpdateAddressMapInfo deserialize(string serpkt);

};
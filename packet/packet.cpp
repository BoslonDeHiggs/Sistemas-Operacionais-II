#include "packet.hpp"
#include <queue>
#include <netdb.h>
Packet::Packet(uint16_t type, uint16_t seqn, uint16_t length, time_t timestamp, string name, string _payload){
    this->type = type;
    this->seqn = seqn;
    this->length = length;
    this->timestamp = timestamp;
    this->name = name;
    this->_payload = _payload;
}

string Packet::serialize(){
    stringstream aux;

    aux << this->type      << ' ';
    aux << this->seqn      << ' ';
    aux << this->length    << ' ';
    aux << this->timestamp << ' ';
    aux << this->name      << ' ';
    aux << this->_payload;

    string str = aux.str();

    return str;
}

Packet Packet::deserialize(string serpkt){
    vector<string> tokens;

    stringstream tokenizer(serpkt);

    string message;

    for (int i = 0; i < NUM_PARAM; i++){
        getline(tokenizer, message, ' ');
        tokens.push_back(message);
    }

    getline(tokenizer, message);

    Packet pkt(stoi(tokens[0]), stoi(tokens[1]), stoi(tokens[2]), (time_t)stoi(tokens[3]), tokens[4], message);

    return pkt;
}

string serialize_addr(sockaddr_in addr){
    uint16_t port = (uint16_t)addr.sin_port;
    
    stringstream aux;
    char ip[INET_ADDRSTRLEN];    
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    aux << ip   << ' ';
    aux << port << ' ';
    string str = aux.str();
    return str;
}

sockaddr_in deserialize_addr(string serpkt){

    stringstream ss(serpkt);
    char ip[INET_ADDRSTRLEN];    
    uint16_t port;
    sockaddr_in addr;
    ss >> ip >> port;

    inet_pton(AF_INET, ip, &(addr.sin_addr));

    struct hostent *host = gethostbyname(ip);
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = *((struct in_addr *)host->h_addr);
    addr.sin_port = htons(port);
    bzero(&(addr.sin_zero), 8);
    
    return addr;
}

ReplicaInfo::ReplicaInfo(int id, uint16_t port, in_addr ip){

}
string ReplicaInfo::serialize(){
    stringstream aux;
    char _ip[INET_ADDRSTRLEN];    
    inet_ntop(AF_INET, &(this->ip), _ip, INET_ADDRSTRLEN);
    aux << this->id    << ' ';
    aux << this->port  << ' ';
    aux << _ip   << ' ';
    string str = aux.str();

    return str;
}
ReplicaInfo ReplicaInfo::deserialize(string serpkt){
    vector<string> tokens;

    stringstream tokenizer(serpkt);

    string message;

    for (int i = 0; i < NUM_PARAM_REPLICA_INFO; i++){
        getline(tokenizer, message, ' ');
        tokens.push_back(message);
    }

    getline(tokenizer, message);
    sockaddr_in addr;
    inet_pton(AF_INET, tokens[2].c_str(), &(addr.sin_addr));

    ReplicaInfo pkt(stoi(tokens[0]), stoi(tokens[1]),addr.sin_addr);

    // cout << pkt.type << ' ' << pkt.seqn << ' ' << pkt.length << ' ' << pkt.timestamp << ' ' << pkt._payload << endl;

    return pkt; 
}

UpdateReplicasInfo::UpdateReplicasInfo(int leader_id, int number_of_replicas, map<int,sockaddr_in> replicas): 
    leader_id(leader_id), number_of_replicas(number_of_replicas), replicas(replicas){;}

string UpdateReplicasInfo::serialize() {
    stringstream ss;
    ss << leader_id << " " << number_of_replicas << " ";
    for (const auto& rep : replicas) {
        ss << rep.first << ":" << serialize_addr(rep.second) << ";";
    }
    return ss.str();
}

UpdateReplicasInfo UpdateReplicasInfo::deserialize(string serpkt) {
    stringstream ss(serpkt);
    string token;
    getline(ss, token, ' ');
    cerr << "Aaaa";
    int leader_id = stoi(token);
    getline(ss, token, ' ');
    cerr<<"CQWe";
    int number_of_replicas = stoi(token);
    map<int, sockaddr_in> replicas;
    while (getline(ss, token, ';')) {
        stringstream ss_replica(token);
        string rep_token;
        getline(ss_replica, rep_token, ':');
        cerr<<"qwe";
        int rep_id = stoi(rep_token);
        getline(ss_replica, rep_token, ' ');
        sockaddr_in addr = deserialize_addr(rep_token);
        replicas[rep_id] = addr;
    }
    return UpdateReplicasInfo(leader_id, number_of_replicas, replicas);
}

UpdateFollowersInfo::UpdateFollowersInfo(map<string,vector<string>>followersMap):followersMap(followersMap){}

string UpdateFollowersInfo::serialize(){
    stringstream ss;
    for (const auto& entry : this->followersMap) {
        ss << entry.first << ":";
        for (const auto& str : entry.second) {
            ss << str << ",";
        }
        ss << ";";
    }
    return ss.str();
}

UpdateFollowersInfo UpdateFollowersInfo::deserialize(string serpkt){
    map<string, vector<string>> followersMap;
    stringstream ss(serpkt);
    string token;
    while (getline(ss, token, ';')) {
        stringstream ss_entry(token);
        string key;
        getline(ss_entry, key, ':');
        vector<string> values;
        string value;
        while (getline(ss_entry, value, ',')) {
            values.push_back(value);
        }
        followersMap[key] = values;
    }
    return followersMap;
}

UpdateMessageQueueInfo::UpdateMessageQueueInfo(map<string, queue<Packet>> client_packet_queue) : client_packet_queue(client_packet_queue) {}


string UpdateMessageQueueInfo::serialize() {
    stringstream ss;
    for (auto entry : client_packet_queue) {
        ss << entry.first << ":";
        while (!entry.second.empty()) {
            Packet pkt = entry.second.front();
            cerr<<"Eh esse";
            ss << pkt.serialize() << ",";
            entry.second.pop();
        }
        ss << ";"; 
    }
    return ss.str();
}

UpdateMessageQueueInfo UpdateMessageQueueInfo::deserialize(string serpkt) {
    map<string, queue<Packet>> messageQueue;
    stringstream ss(serpkt);
    string token;
    while (getline(ss, token, ';')) {
        stringstream ss_entry(token);
        string key;
        getline(ss_entry, key, ':');
        queue<Packet> packets;
        string pkt_str;
        while (getline(ss_entry, pkt_str, ',')) {
            packets.push(Packet::deserialize(pkt_str));
        }
        messageQueue[key] = packets;
    }
    return UpdateMessageQueueInfo(messageQueue);
}


UpdateAddressMapInfo::UpdateAddressMapInfo(map<string, vector<sockaddr_in>> addressMap)
    : addressMap(addressMap) {}

string UpdateAddressMapInfo::serialize() {
    stringstream ss;
    for (const auto& entry : this->addressMap) {
        ss << entry.first << ":";
        for (const auto& addr : entry.second) {
            ss << serialize_addr(addr) << ",";
        }
        ss << ";";
    }
    return ss.str();
}

UpdateAddressMapInfo UpdateAddressMapInfo::deserialize(string serpkt) {
    map<string, vector<sockaddr_in>> addressMap;
    stringstream ss(serpkt);
    string token;
    while (getline(ss, token, ';')) {
        stringstream ss_entry(token);
        string key;
        getline(ss_entry, key, ':');
        vector<sockaddr_in> addrs;
        string addr_str;
        while (getline(ss_entry, addr_str, ',')) {
            addrs.push_back(deserialize_addr(addr_str));
        }
        addressMap[key] = addrs;
    }
    return UpdateAddressMapInfo(addressMap);
}

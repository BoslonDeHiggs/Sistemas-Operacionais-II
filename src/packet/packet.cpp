#include "packet.hpp"

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

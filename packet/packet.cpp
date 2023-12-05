#include "packet.hpp"

Packet::Packet(uint16_t type, uint16_t seqn, uint16_t length, uint16_t timestamp, string _payload){
    this->type = type;
    this->seqn = seqn;
    this->length = length;
    this->timestamp = timestamp;
    this->_payload = _payload;
}

string Packet::serialize(){
    stringstream aux;

    aux << this->type      << ' ';
    aux << this->seqn      << ' ';
    aux << this->length    << ' ';
    aux << this->timestamp << ' ';
    aux << this->_payload;

    string str = aux.str();

    return str;
}

Packet Packet::deserialize(string serpkt){
    vector<string> tokens;

    stringstream tokenizer(serpkt);

    string aux_str;

    for (int i = 0; i < 4; i++){
        getline(tokenizer, aux_str, ' ');
        tokens.push_back(aux_str);
    }

    getline(tokenizer, aux_str);

    // cout << "Aux_str.c_str(): " << aux_str.c_str() << endl;

    Packet pkt(stoi(tokens[0]), stoi(tokens[1]), stoi(tokens[2]), stoi(tokens[3]), aux_str);

    // cout << pkt.type << ' ' << pkt.seqn << ' ' << pkt.length << ' ' << pkt.timestamp << ' ' << pkt._payload << endl;

    return pkt;
}

#include "serdes.hpp"

using namespace std;

string serpkt(packet pkt){
    stringstream aux;
    aux << pkt.type      << ' ';
    aux << pkt.seqn      << ' ';
    aux << pkt.length    << ' ';
    aux << pkt.timestamp << ' ';
    aux << pkt._payload;

    string str = aux.str();

    return str;
}

packet despkt(string dtgram){
    vector<string> tokens;

    stringstream tokenizer(dtgram);

    string aux_str;

    while(getline(tokenizer, aux_str, ' ')){
        tokens.push_back(aux_str);
    }

    packet pkt;

    pkt.type      = stoi(tokens[0]);
    pkt.seqn      = stoi(tokens[1]);
    pkt.length    = stoi(tokens[2]);
    pkt.timestamp = stoi(tokens[3]);

    stringstream aux_stream;

    for (unsigned int i = 4; i < tokens.size(); i++){
        aux_stream << tokens[i] << ' ';
    }
    aux_stream << "\0";

    aux_str = aux_stream.str();

    pkt._payload = aux_str.c_str();

    return pkt;
}
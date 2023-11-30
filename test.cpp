#include <iostream>
#include "serdes.hpp"

int main(){
    string str = "Hello World!";
    string aux_str;

    packet pkt;
    packet aux_pkt;

    pkt.type = DATA;
    pkt.seqn = 10;
    pkt.length = str.length();
    pkt.timestamp = time(NULL);
    pkt._payload = str.c_str();

    aux_str = serpkt(pkt);

    cout << aux_str << endl;

    aux_pkt = despkt(aux_str);

    cout << aux_pkt.type << ' ' << aux_pkt.seqn << ' ' << aux_pkt.length << ' ' << aux_pkt.timestamp << ' ' << aux_pkt._payload << endl;
}
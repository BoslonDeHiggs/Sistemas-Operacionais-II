#pragma once

#include <cstdint>
#include <string>
#include <bits/stdc++.h>

#define MSG_SIZE 128
#define NUM_PARAM 5

#define SEND 100

using namespace std;

class Packet{
    public:
        uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
        uint16_t seqn; //Número de sequência
        uint16_t length; //Comprimento do payload
        uint16_t timestamp; // Timestamp do dado
        string name; //Nome do remetente
        string _payload; //Dados da mensagem

        Packet(uint16_t type, uint16_t seqn, uint16_t length, uint16_t timestamp, string name, string _payload);

        string serialize();

        static Packet deserialize(string serpkt);

    private:
};
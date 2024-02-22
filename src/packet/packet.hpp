#pragma once

#include <cstdint>
#include <string>
#include <bits/stdc++.h>

#define MSG_SIZE 128
#define NUM_PARAM 5

#define SEND 100
#define LOGIN 200
#define FOLLOW 300
#define EXIT 400
#define DISCOV_MSG 450
#define ELECTION_START 500
#define ELECTION_ANSWER 600
#define ELECTION_COORDINATOR 700
#define LINK_REPLICA 900 // will link the new replica to the primary replica (just add to primary map of replicas)
#define UPDATE_REPLICA 1000 // the replica which receives this will be updated with some data in the payload
#define HEARTBEAT 1200
#define NEW_SERVER 1300
#define NEW_LEADER 1400
#define DATABASE 1500
#define LEADER_CHECK 1600

using namespace std;

class  Packet{
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
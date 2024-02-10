
#include "../server/server.hpp"

#define ERROR_ID -1
#define ELECTION_TIMEOUT 2
class ReplicaManager: public Server{
    public:
        ReplicaManager(uint16_t replica_port, const char* leader_ip, uint16_t leader_port);
        void start_election();

        void handle_election();
        
        void become_leader();
    private:
        int id;
        uint16_t port;
        int leader_id;
        sockaddr_in leader_addr; //inet_ntoa(leader_addr.sin_addr) to return the ip
        std::map<int, sockaddr_in> replicas; // map id -> addr
        bool election_started; 
        bool got_answer;
};
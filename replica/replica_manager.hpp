
#include "../server/server.hpp"

#define ERROR_ID -1
#define ELECTION_TIMEOUT 2

class ReplicaManager: public Server{
    public:
        ReplicaManager(uint16_t replica_port, int is_leader);
        
        void start_election();

        void handle_election();
        
        void become_leader();

        void process_multicast();

        void update_replica_on_replica_address(sockaddr_in replica_addr);

        void call_processThread_multicast();
    private:
        void handle_link_replica(Packet packet);
        void handle_update_replica(Packet packet);
        void handle_election_coordinator(Packet packet);

        int id;
        uint16_t port;
        int leader_id;
        std::map<int, sockaddr_in> replicas; // map id -> addr
        bool election_started; 
        bool got_answer;
};
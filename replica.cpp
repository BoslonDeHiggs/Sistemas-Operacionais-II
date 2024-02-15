#include "replica/replica_manager.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>/* Replica entrypoint */

using namespace std;

int main(int argc, char **argv)
{
    // Parse command line input
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << "<replica-port> <is-leader>" << std::endl;
        return 1;
    }

    try
    {
        // Create an instance of Replica
        ReplicaManager replica(atoi(argv[1]),atoi(argv[2]));

        // Start listening for connections
        replica.init_database();
        replica.call_listenThread();
        replica.call_processThread();
        replica.call_sendThread();

    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << '\n';
    }

    // End
    std::cout << "Replica " << argv[3] << " stopped safely." << std::endl;
    return 0;
}
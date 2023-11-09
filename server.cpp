#include <iostream>
#include <string>
#include <vector>
#include "client.cpp"

class Server
{
public:
    Server()
    {
    }

private:
    vector<Client> clients;
};
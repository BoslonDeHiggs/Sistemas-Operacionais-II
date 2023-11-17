#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include "client.h"

using namespace std;
class Server
{
public:
    Server()
    {
    }

    int createAccount(Client client_in)
    {
        int id = rand();
        client_in.setId(id);
        client_vect.push_back(client_in);

        return id;
    }

    vector<Client> getFollowers(int id)
    {
        return client_vect[id].getFollowers();
    }

    void addFollower(int id, Client fol_in)
    {
        for (Client &it : client_vect)
        {
            if (it.getId() == id)
            {
                it.addFollower(fol_in);
            }
        }
    }

private:
    vector<Client> client_vect;
};
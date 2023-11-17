#include "client.h"
#include "server.cpp"
#include <iostream>

using namespace std;

int main()
{
    Server server;
    Client client1("Afonso"), client2("Jesus");

    int id1 = server.createAccount(client1);
    client1.setId(id1);

    int id2 = server.createAccount(client2);
    client2.setId(id2);

    client1.addFollower(client2);
    server.addFollower(client1.getId(), client2);

    vector<Client> vect = server.getFollowers(client1.getId());

    cout << vect[0].getName() << ", " << vect[0].getId() << endl;

    return 0;
}
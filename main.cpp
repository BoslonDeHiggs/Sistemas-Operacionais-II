#include "client.cpp"
#include <iostream>

using namespace std;

int main(){
    Client client1("Afonso", 2345), client2("Jesus", 6789);

    client1.follow(client2);
    client2.follow(client1);

    cout << client1.getFollowById(6789).getName() << endl;

    return 0;
}
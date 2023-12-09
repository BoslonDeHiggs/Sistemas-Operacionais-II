#pragma once

#include <fstream>
#include <map>
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

typedef struct user{
    string name;
    sockaddr_in address;
}user;

class Database
{
public:
    int open();

    bool contains(string username);

    void add_user(string username);

    vector<user> get_followers(string username);

    bool add_followers(string username, user follower);

    void close();

private:
    map<string, vector<user>> data;

    fstream file;

};
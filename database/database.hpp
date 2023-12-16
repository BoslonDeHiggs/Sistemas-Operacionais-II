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
    map<string, vector<sockaddr_in>> addressMap;

    int open();

    bool contains(string username);

    bool is_logged_in(string username, sockaddr_in address);

    void add_user(string username);

    vector<string> get_followers(string username);

    bool add_follower(string username, string follower);

    void login(string username, sockaddr_in address);

    void sign_up(string username, sockaddr_in address);

    void exit(string username, sockaddr_in address);

    void close();

private:
    map<string, vector<string>> followersMap;

    fstream file;

};
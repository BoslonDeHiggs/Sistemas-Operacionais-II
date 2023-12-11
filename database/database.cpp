#include "database.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <string>

using namespace std;

int Database::open(){
    this->file.open("database/database.txt", ios::app);
    if(file.is_open()){
        return 0;
    }
    else return -1;
}

bool Database::contains(string username){
    map<string, vector<user>>::iterator it;
    it = data.find(username);
    if(it == data.end()){
        return false;
    }
    else return true;
}

void Database::add_user(string username){
    vector<user> aux;
    data.insert({username, aux});
}

vector<user> Database::get_followers(string username){
    if(this->contains(username)){
        map<string, vector<user>>::iterator it;
        it = data.find(username);
        return it->second;
    }
    else{
        vector<user> null;
        return null;
    }
}

bool Database::add_followers(string username, user follower){
    if(this->contains(username)){
        map<string, vector<user>>::iterator it;
        it = data.find(username);
        it->second.push_back(follower);
        return true;
    }
    else return false;
}

void Database::close(){
    this->file.close();
}
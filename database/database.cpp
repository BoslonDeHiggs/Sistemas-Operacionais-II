#include "database.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <algorithm>

using namespace std;

bool operator==(const sockaddr_in& lhs, const sockaddr_in& rhs) {
    return memcmp(&lhs, &rhs, sizeof(sockaddr_in)) == 0;
}

int Database::open(){
    this->file.open("database/database.txt", ios::app);
    if(file.is_open()){
        return 0;
    }
    else return -1;
}

void Database::login(string username, sockaddr_in address){
    map<string, vector<sockaddr_in>>::iterator it_am;
    it_am = addressMap.find(username);
    if(it_am == addressMap.end()){
        vector<sockaddr_in> aux{address};
        addressMap.insert({username, aux});
    }
    else{
        if(find(it_am->second.begin(), it_am->second.end(), address) == it_am->second.end())
            it_am->second.push_back(address);
    }
}

void Database::sign_up(string username, sockaddr_in address){
    map<string, vector<string>>::iterator it_fm;
    it_fm = followersMap.find(username);
    if(it_fm == followersMap.end()){
        vector<string> aux{};
        followersMap.insert({username, aux});
    }

    map<string, vector<sockaddr_in>>::iterator it_am;
    it_am = addressMap.find(username);
    if(it_am == addressMap.end()){
        vector<sockaddr_in> aux{address};
        addressMap.insert({username, aux});
    }
    else{
        if(find(it_am->second.begin(), it_am->second.end(), address) == it_am->second.end())
            it_am->second.push_back(address);
    }
}

void Database::exit(string username, sockaddr_in address){
    map<string, vector<sockaddr_in>>::iterator it_am;
    it_am = addressMap.find(username);
    if(it_am != addressMap.end()){
        vector<sockaddr_in>::iterator it = find(it_am->second.begin(), it_am->second.end(), address);
        if(it != it_am->second.end())
            it_am->second.erase(it);
    }
}

bool Database::contains(string username){
    map<string, vector<string>>::iterator it;
    it = followersMap.find(username);
    if(it == followersMap.end()){
        return false;
    }
    else return true;
}

bool Database::is_logged_in_addr(string username, sockaddr_in address){
    map<string, vector<sockaddr_in>>::iterator it_am;
    it_am = addressMap.find(username);
    if(it_am == addressMap.end()){
        return false;
    }
    else{
        vector<sockaddr_in>::iterator it = find(it_am->second.begin(), it_am->second.end(), address);
        if(it != it_am->second.end())
            return true;
        else
            return false;
    }
}

bool Database::is_logged_in(string username){
    map<string, vector<sockaddr_in>>::iterator it_am;
    it_am = addressMap.find(username);
    if(it_am == addressMap.end()){
        return false;
    }
    else{
        return true;
    }
}

vector<string> Database::get_followers(string username){
    if(this->contains(username)){
        map<string, vector<string>>::iterator it;
        it = followersMap.find(username);
        return it->second;
    }
    else{
        vector<string> null{};
        return null;
    }
}

bool Database::add_follower(string username, string follower){
    if(this->contains(username)){
        map<string, vector<string>>::iterator it;
        it = followersMap.find(username);
        if(find(it->second.begin(), it->second.end(), follower) == it->second.end()){
            it->second.push_back(follower);
            return true;
        }
        else return false;
    }
    else return false;
}

void Database::write(){
    for(auto index : followersMap){
        file << index.first << ":";
        for(auto name : index.second){
            file << name << ";";
        }
        file << endl;
    }
}

void Database::close(){
    this->file.close();
}
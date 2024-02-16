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
    auto it = addressMap.find(username);
    // Verifica se o usuário existe no addressMap e tem pelo menos um endereço associado.
    return (it != addressMap.end() && !it->second.empty());
    // map<string, vector<sockaddr_in>>::iterator it_am;
    // it_am = addressMap.find(username);
    // if(it_am == addressMap.end()){
    //     return false;
    // }
    // else{
    //     return true;
    // }
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
    // Abre o arquivo para escrita, limpando o conteúdo existente
    std::ofstream file("database/database.txt", std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita" << std::endl;
        return;
    }

    // Escreve cada entrada do map followersMap no arquivo
    for (const auto& pair : followersMap) {
        file << pair.first << ":";
        for (const auto& follower : pair.second) {
            file << follower << ";";
        }
        file << std::endl;
    }

    // Fecha o arquivo
    file.close();
}

void Database::read() {
    std::ifstream file("database/database.txt");

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para leitura" << std::endl;
        return;
    }

    std::string line;
    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string username, followers;
        getline(ss, username, ':');
        getline(ss, followers);

        followersMap[username] = vector<string>();

        std::stringstream ss_followers(followers);
        std::string follower;
        while (getline(ss_followers, follower, ';')) {
            if (!follower.empty()) {
                followersMap[username].push_back(follower);
            }
        }
    }

    file.close();
}

void Database::close(){
    this->file.close();
}

void Database::storeMessageForOfflineUser(const string& username, const Packet& pkt) {
    messageQueue[username].push(pkt);
}
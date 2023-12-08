#include "database.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <string>

using namespace std;

int Database::open(){
    this->file.open("database.txt", ios::app);
    if(file.is_open()){
        return 0;
    }
    else return -1;
}

string Database::find(string user){
    string line;
    bool found = false;
    while(getline(this->file, line) && !found){
        string name;
        stringstream aux(line);
        getline(aux, name, ':');
        if(name == user){
            found = true;
            return line;
        }
    }
    return "";
}

void Database::close(){
    this->file.close();
}
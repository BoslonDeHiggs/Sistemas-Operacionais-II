#include "database.hpp"
#include <iostream>

int Database::open(){
    this->file.open("database.txt", ios::app);
    if(file.is_open()){
        return 0;
    }
    else return -1;
}

int Database::find(string user){
    return 0;
}

void Database::close(){
    this->file.close();
}
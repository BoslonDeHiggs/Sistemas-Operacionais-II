#pragma once

#include <fstream>

using namespace std;

class Database
{
public:
    int open();

    int find(string user);

    void close();

private:
    fstream file;

};
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <iostream>

using namespace std;

void func1(){
    while(true){
        cout << "sou a primeira thread" << endl;
        sleep(1);
    }
}

void func2(){
    while(true){
        cout << "sou a segunda thread" << endl;
        sleep(1);
    }
}

int main(){
    thread first(func1);
    thread second(func2);

    first.join();
    second.join();

    return 0;
}
#include "format.hpp"

string format_time(time_t time){
    char timeStr[100];

    strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M:%S", localtime(&time));

    string timestamp(timeStr);

    return timestamp;
}

void print_rcv_msg(time_t time, string name, string msg){
    string timestamp = format_time(time);

    if(name == "SERVER"){
        cout << YELLOW << "(" << timestamp << ") " << BLUE << "[%] " << name << "~ " << msg << RESET << endl;
    }
    else{
        cout << YELLOW << "(" << timestamp << ") " << BRIGHT_CYAN << "[!] " << name << "~ " << RESET << msg << endl;
    }
}

void print_send_msg(time_t time, string name, string code, string msg){
    string timestamp = format_time(time);
    
    cout << WRITEOVER << YELLOW << "(" << timestamp << ") " << MAGENTA << "[#] " << name << "~ " << GREEN << code << " " << RESET << msg << endl;
}

void print_client_msg(string msg){
    stringstream formated_msg;

    string timestamp = format_time(time(NULL));

    cout << endl << WRITEOVER << YELLOW << "(" << timestamp << ") " << BRIGHT_BLACK << "[&] CLIENT~ " << msg << RESET << endl;
}

void print_server_ntf(time_t time, string msg, string name){
    stringstream formated_msg;

    string timestamp = format_time(time);

    cout << YELLOW << "(" << timestamp << ") " << BLUE << "[%] SERVER~ " << msg << " " << name << RESET << endl;
}

void print_server_follow_ntf(time_t time, string msg, string name_1, string name_2){
    stringstream formated_msg;

    string timestamp = format_time(time);

    cout << YELLOW << "(" << timestamp << ") " << BLUE << "[%] SERVER~ " << name_1 << " " << msg << " " << name_2 << RESET << endl;
}

void print_error_msg(string msg){
    stringstream formated_msg;

    string timestamp = format_time(time(NULL));

    cerr << YELLOW << "(" << timestamp << ") " << RED << "[¬] ERROR~ " << msg << RESET << endl;
}

void save_cursor(){
    cout << SAVE;
}

void restore_cursor(){
    cout << RESTORE;
}
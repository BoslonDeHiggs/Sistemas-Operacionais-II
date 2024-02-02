#include "../database/database.hpp"
#include "../server/server.hpp"
#include "../packet/packet.hpp"
#include "../client/client.hpp"

#define PORT 4000
const char *ip;

#define CANT_FOLLOW_ERROR "Can't follow self"
#define DEFAULT_ERROR "Something went wrong"
#define MORE_THAN_TWO_SESSIONS_ERROR "There are two other sessions already active"

#define MAX_ITERATIONS 50000
#define STDOUT_BUFFER_SIZE 1024
#define REPLACE_OF_STDOUT "stdout.txt"
#define REPLACE_OF_STDERR "stderr.txt"

int stdoutSave;
int stderrSave;
char outputBuffer[STDOUT_BUFFER_SIZE];
char stderrBuffer[STDOUT_BUFFER_SIZE];

using namespace std;

void replace_stderr()
{
    fflush(stderr); //clean everything first
    stderrSave = dup(STDERR_FILENO); //save the stdout state
    freopen(REPLACE_OF_STDERR, "a", stderr); //redirect stdout to null pointer
    setvbuf(stderr, stderrBuffer, _IOFBF, STDOUT_BUFFER_SIZE); //set buffer to stdout
}
void restore_stderr(){
    freopen(REPLACE_OF_STDERR, "a", stderr); //redirect stdout to null again
    dup2(stderrSave, STDERR_FILENO); //restore the previous state of stdout
    setvbuf(stderr, NULL, _IONBF, STDOUT_BUFFER_SIZE); //disable buffer to print to screen instantly
}
void replace_stdout()
{
    fflush(stdout); //clean everything first
    stdoutSave = dup(STDOUT_FILENO); //save the stdout state
    freopen(REPLACE_OF_STDOUT, "a", stdout); //redirect stdout to null pointer
    setvbuf(stdout, outputBuffer, _IOFBF, STDOUT_BUFFER_SIZE); //set buffer to stdout
}
void restore_stdout()
{
    freopen(REPLACE_OF_STDOUT, "a", stdout); //redirect stdout to null again
    dup2(stdoutSave, STDOUT_FILENO); //restore the previous state of stdout
    setvbuf(stdout, NULL, _IONBF, STDOUT_BUFFER_SIZE); //disable buffer to print to screen instantly
}

Client setup_client(string name){
    Client client(name);
    client.connect_to_udp_server(ip, PORT);
    client.login();
    client.call_listenThread();
    return client;
}

void setup_server(){

    Server server(PORT);

    server.init_database();
    server.open_udp_connection(PORT);
    server.call_processThread();
    server.call_listenThread();

}
int cant_follow_itself(){
    string errorMessage(CANT_FOLLOW_ERROR);
    string name = "nikolas";
    Client client = setup_client(name);
    string username = "@" + name;
    client.follow(username);
    bool foundMessage = false;
    for(int i = 0; i < MAX_ITERATIONS; i++){
        string buffer (outputBuffer);
        if (buffer.find(errorMessage) != std::string::npos) {
            foundMessage = true;
            break;
        }  
    }
    if(globalClientPointer != nullptr){
        client.sendExit();
    }
    if (foundMessage) return 0;
    else return 1;
}

int cant_follow_nonexistent_user(){
    string errorMessage(DEFAULT_ERROR);
    string name("nikolas");
    Client client = setup_client(name);
    string nonexistentUser = "_"; //always nonexistent because it doesnt have prefix '@'
    client.follow(nonexistentUser);
    bool foundMessage = false;
    for(int i = 0; i < MAX_ITERATIONS; i++){
        string buffer (outputBuffer);
        if (buffer.find(errorMessage) != std::string::npos) {
            foundMessage = true;
            break;
        }  
    }
    if(globalClientPointer != nullptr)
        client.sendExit();

    if (foundMessage) return 0;
    else return 1;
}
int message_sent_correctly(){
    string message_to_send("My test_message");
    string nameA("nikolas");
    string nameB("jorge");
    Client clientA = setup_client(nameA);
    Client clientB = setup_client(nameB);

    clientA.follow("@"+nameB);
    clientB.send_message(message_to_send);
    bool foundMessage = false;
    for(int i = 0; i < MAX_ITERATIONS; i++){
        string buffer (outputBuffer);
        if (buffer.find(message_to_send) != std::string::npos) {
            foundMessage = true;
            break;
        }  
    }
    if(globalClientPointer != nullptr){
        clientA.sendExit();
        clientB.sendExit();
    }
    if (foundMessage) return 0;
    else return 1;
}
int message_is_sent_to_offline_user(){
    string message_to_send("My other test_message");
    string nameA("nikolas");
    string nameB("jorge");
    Client clientA = setup_client(nameA);
    Client clientB = setup_client(nameB);

    clientA.follow("@"+nameB);
    if(globalClientPointer != nullptr)
        clientA.sendExit();    
    sleep(5);
    clientB.send_message(message_to_send);
    sleep(5);
    clientA = setup_client(nameA);
    bool foundMessage = false;
    for(int i = 0; i < MAX_ITERATIONS; i++){
        string buffer (outputBuffer);
        if (buffer.find(message_to_send) != std::string::npos) {
            foundMessage = true;
            break;
        }  
    }
    if(globalClientPointer != nullptr){
        clientA.sendExit();
        clientB.sendExit();
    }
    if (foundMessage) return 0;
    else return 1;
}
int cant_have_more_than_two_sessions(){
    
    string errorMessage(MORE_THAN_TWO_SESSIONS_ERROR);
    string nameA("nikolas");
    Client clientA = setup_client(nameA);
    Client clientB = setup_client(nameA);
    Client clientC = setup_client(nameA);
    bool foundMessage = false;
    for(int i = 0; i < MAX_ITERATIONS; i++){
        string buffer (outputBuffer);
        if (buffer.find(errorMessage) != std::string::npos) {
            foundMessage = true;
            break;
        }  
    }
    if(globalClientPointer != nullptr){
        clientA.sendExit();
        clientB.sendExit();
        clientC.sendExit();
    }
    if (foundMessage) return 0;
    else return 1;
}


int main(int argc, char* argv[]){

    thread server_thread(setup_server);
    server_thread.detach();
    if(argc != 2){
        print_error_msg("Correct usage is: ./test <server_ip>");
        return 0;
    }
    ip = argv[1];

    replace_stdout();
    replace_stderr();
    vector<int> test_results;
    test_results.push_back(cant_follow_itself());
    test_results.push_back(cant_follow_nonexistent_user());
    test_results.push_back(message_sent_correctly());
    test_results.push_back(message_is_sent_to_offline_user());
    test_results.push_back(cant_have_more_than_two_sessions());
    sleep(10);
    remove(REPLACE_OF_STDERR);
    remove(REPLACE_OF_STDOUT);
    restore_stderr();
    fflush(stderr);
    int final_result = 0;
    for (size_t i = 0; i<test_results.size(); i++){
        if(test_results[i] != 0){
            string test_error_message = "Error occurred on test ";
            test_error_message += to_string(i);
            print_error_msg(test_error_message);
        }
        final_result+=test_results[i];
    }
    if(final_result == 0)
        print_success_msg("All tests passed");
    return 0;
}


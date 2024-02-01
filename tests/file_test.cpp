#include "../database/database.hpp"
#include "../server/server.hpp"
#include "../packet/packet.hpp"
#include "../client/client.hpp"

#define PORT 4000
const char *ip;

#define CANT_FOLLOW_ERROR_MESSAGE "Can't follow self"
#define DEFAULT_ERROR_MESSAGE "Something went wrong"


#define STDOUT_BUFFER_SIZE 1024
int stdoutSave;
char outputBuffer[STDOUT_BUFFER_SIZE];

using namespace std;
void replace_stdout()
{
    fflush(stdout); //clean everything first
    stdoutSave = dup(STDOUT_FILENO); //save the stdout state
    freopen("NUL", "a", stdout); //redirect stdout to null pointer
    setvbuf(stdout, outputBuffer, _IOFBF, 1024); //set buffer to stdout
}

void restore_stdout()
{
    freopen("NUL", "a", stdout); //redirect stdout to null again
    dup2(stdoutSave, STDOUT_FILENO); //restore the previous state of stdout
    setvbuf(stdout, NULL, _IONBF, STDOUT_BUFFER_SIZE); //disable buffer to print to screen instantly
}

Client setup_client(string name){
    string name("nikolas");
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
    string cantFollowMessage(CANT_FOLLOW_ERROR_MESSAGE);
    string name = "nikolas";
    Client client = setup_client(name);
    string username = "@" + name;
    replace_stdout();
    client.follow(username);
    sleep(5);
    restore_stdout();
    if(globalClientPointer != nullptr)
        client.sendExit();
    string buffer (outputBuffer);
    if (buffer.find(cantFollowMessage) != std::string::npos) {
        return 0;
    }    
    return 1;
}

int cant_follow_nonexistent_user(){
    string errorMessage(DEFAULT_ERROR_MESSAGE);
    string name("nikolas");
    Client client = setup_client(name);
    string nonexistentUser = "_"; //always nonexistent because it doesnt have prefix '@'
    replace_stdout();
    client.follow(nonexistentUser);
    sleep(5);
    restore_stdout();
    if(globalClientPointer != nullptr)
        client.sendExit();
    string buffer (outputBuffer);
    if (buffer.find(DEFAULT_ERROR_MESSAGE) != std::string::npos) {
        return 0;
    }    
    return 1;
}
int message_sent_correctly(){

    return 0;
    }
int message_is_sent_to_offline_user(){return 0;}
int cant_have_more_than_two_sessions(){return 0;}


int main(int argc, char* argv[]){

    thread server_thread(setup_server);
    server_thread.detach();
    if(argc != 2){
        print_error_msg("Correct usage is: ./test <server_ip>");
        return 0;
    }
    ip = argv[1];

    vector<int> test_results;
    test_results.push_back(cant_follow_itself());
    test_results.push_back(cant_follow_nonexistent_user());
    test_results.push_back(message_sent_correctly());
    test_results.push_back(message_is_sent_to_offline_user());
    test_results.push_back(cant_have_more_than_two_sessions());
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


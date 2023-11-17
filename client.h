#include <string>
#include <vector>

using namespace std;

class Client
{
public:
    Client(string name_in);

    string getName();

    void addFollower(Client fol_in);

    vector<Client> getFollowers();

    void setId(int id_in);
    
    int getId();

private:
    int id;
    string name;
    vector<Client> followers;
};
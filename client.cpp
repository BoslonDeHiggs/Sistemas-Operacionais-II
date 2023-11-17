#include <string>
#include <vector>

using namespace std;
class Client
{
public:
    Client(string name_in)
    {
        name = name_in;
    }

    string getName()
    {
        return name;
    }

    void addFollower(Client fol_in)
    {
        followers.push_back(fol_in);
    }

    vector<Client> getFollowers()
    {
        return followers;
    }

    void setId(int id_in)
    {
        id = id_in;
    }

    int getId()
    {
        return id;
    }

private:
    int id;
    string name;
    vector<Client> followers;
};
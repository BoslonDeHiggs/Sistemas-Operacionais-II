#include <string>
#include <vector>

using namespace std;
class Client
{
public:
    Client(string name_in, int id_in)
    {
        id = id_in;
        name = name_in;
    }

    int getId()
    {
        return id;
    }

    string getName()
    {
        return name;
    }

    Client getFollowById(int id_in)
    {
        for (int i = 0; i < following.size(); i++)
        {
            if (following[i].id == id_in)
            {
                return following[i];
            }
        }

        return following[0];
    }

    void follow(Client user)
    {
        following.push_back(user);
    }

private:
    int id;
    string name;
    vector<Client> following;
};
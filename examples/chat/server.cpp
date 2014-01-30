
#include <iostream>
#include <sstream>
#include <iomanip>
#include <spdr/spdr.h>

#include "defines.h"

int main()
{
    std::map<unsigned int, std::string> users;

    spdr::Node node(CHAT_PROTOCOL_ID, false);    
    
    node.on_connect([&] (unsigned int peer) 
    {
        std::cout << "INFO: Peer " << peer << " connected." << std::endl;
    });
    node.on_disconnect([&] (unsigned int peer) 
    {
        auto i = users.find(peer);
        if (i != users.end())
        {
            std::stringstream msg;
            msg << i->second << " disconnected.";
            node.broadcast(SERVER_MESSAGE, "SERVER", msg.str());
            users.erase(i);
        }
        std::cout << "INFO: Peer " << peer << " disconnected." << std::endl;
    });
    
    node.on_message<std::string>(JOIN_MESSAGE, [&] (unsigned int peer, std::string name) 
    {
        auto i = users.find(peer);
        if (i == users.end())
        {
            users[peer] = name;            
            std::stringstream msg;
            msg << name << " joined.";
            node.broadcast(SERVER_MESSAGE, "SERVER", msg.str());
        }
        else
        {
            std::cout << "WARNING: Peer " << peer << " has already sent a join message." << std::endl;
        }
    });
    
    node.on_message<std::string>(CHAT_MESSAGE, [&] (unsigned int peer, std::string text) 
    {
        auto i = users.find(peer);
        if (i != users.end())
        {
            std::string name = i->second;
            node.broadcast(SERVER_MESSAGE, name, text);
            std::cout << std::setw(10) << std::left << name << ": " << text << std::endl;
        }
        else
        {
            std::cout << "WARNING: Peer " << peer << " tries to talk but has not yet joined." << std::endl;
        }
    });
    
    node.listen(2001);    
    node.run();    
}

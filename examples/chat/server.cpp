
#include <iostream>
#include <sstream>
#include <spdr/spdr.h>

#include "defines.h"

int main()
{
    std::map<spdr::Peer*, std::string> users;

    spdr::Node node(CHAT_PROTOCOL_ID, CHAT_PROTOCOL_VERSION);    
    
    node.on_message<std::string>(JOIN_MESSAGE, [&] (spdr::Peer* peer, std::string name) 
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
            std::cout << "WARNING: Peer " << peer->get_address() << ":" << peer->get_port() << " has already sent a join message." << std::endl;
        }
    });
    
    node.on_message<std::string>(CHAT_MESSAGE, [&] (spdr::Peer* peer, std::string text) 
    {
        auto i = users.find(peer);
        if (i != users.end())
        {
            std::string name = i->second;
            node.broadcast(SERVER_MESSAGE, name, text);
        }
        else
        {
            std::cout << "WARNING: Peer " << peer->get_address() << ":" << peer->get_port() << " tries to talk but has not yet joined." << std::endl;
        }
    });
    
    node.listen(2001, [&] (spdr::Peer* peer) 
    {
        std::cout << "INFO: Peer " << peer->get_address() << ":" << peer->get_port() << " connected." << std::endl;
    }, 
    [&] (spdr::Peer* peer) 
    {
        auto i = users.find(peer);
        if (i != users.end())
        {
            std::stringstream msg;
            msg << i->second << " disconnected.";
            node.broadcast(SERVER_MESSAGE, "SERVER", msg.str());
            users.erase(i);
        }
        std::cout << "INFO: Peer " << peer->get_address() << ":" << peer->get_port() << " disconnected." << std::endl;
    });
    
    node.run();    
}

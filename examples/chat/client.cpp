
#include "defines.h"

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <spdr/spdr.h>

int main(int argc, char* argv[])
{
    if (argc != 3) 
    {
        std::cerr << "Usage: " << std::endl
                  << " char-client <address> <username> " << std::endl;
        return -1;
    }
    
    std::string address  = argv[1];
    std::string username = argv[2];
    
    spdr::Node node(CHAT_PROTOCOL_ID, CHAT_PROTOCOL_VERSION);
    
    node.on_message<std::string, std::string>(SERVER_MESSAGE, [&] (spdr::Peer* peer, std::string user, std::string text) 
    {
        std::cout << std::setw(10) << user << ":" << text << std::endl;
    });
    
    node.connect(address, 2001, [&] (spdr::Peer* peer) 
    {
        std::cout << "INFO: Connected to " << peer->get_address() << ":" << peer->get_port() << "." << std::endl;
        node.send(peer, JOIN_MESSAGE, username);
        
        //input.read([&] (const std::string& text) {
        //    node.send(peer, CHAT_MESSAGE, text);
        //});        
    }, 
    [&] (spdr::Peer* peer) 
    {
        std::cout << "INFO: Disconnected from " << peer->get_address() << ":" << peer->get_port() << "." << std::endl;
        node.stop();
    });
    
    node.run();
}

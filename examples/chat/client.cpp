
#include "defines.h"

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <spdr/spdr.h>
#include <c9y/c9y.h>

int main(int argc, char* argv[])
{
    spdr::Peer* server = NULL;
    bool running = true;

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
        if (user != username)
        {
            std::cout << std::setw(10) << std::left << user << ": " << text << std::endl;
        }
    });
    
    node.connect(address, 2001, [&] (spdr::Peer* peer) 
    {
        std::cout << "INFO: Connected to " << peer->get_address() << ":" << peer->get_port() << "." << std::endl;
        node.send(peer, JOIN_MESSAGE, username);
        server = peer;
    }, 
    [&] (spdr::Peer* peer) 
    {
        std::cout << "INFO: Disconnected from " << peer->get_address() << ":" << peer->get_port() << "." << std::endl;
        node.stop();
        server = NULL;
        running = false;
    });
    
    // TODO get c9y to do async I/O
    c9y::Thread* io = new c9y::Thread([&] () {
        while (running)
        {
            std::string line;
            std::getline(std::cin, line);
            
            if (server != NULL && ! line.empty())
            {
                node.send(server, CHAT_MESSAGE, line);
            }
        }
    });    
    
    c9y::run();
    
    io->join();
}


#include "defines.h"

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <spdr/spdr.h>

int main(int argc, char* argv[])
{
    bool running = true;

    if (argc != 3) 
    {
        std::cerr << "Usage: " << std::endl
                  << " char-client <address> <username> " << std::endl;
        return -1;
    }
    
    std::string address  = argv[1];
    std::string username = argv[2];
    
    spdr::Node node(CHAT_PROTOCOL_ID);
    
    node.on_disconnect([&] (unsigned int peer) 
    {
        std::cout << "INFO: Disconnected from " << peer << "." << std::endl;
        running = false;
    });
    
    node.on_message<std::string, std::string>(SERVER_MESSAGE, [&] (unsigned int peer, std::string user, std::string text) 
    {
        if (user != username)
        {
            std::cout << std::setw(10) << std::left << user << ": " << text << std::endl;
        }
    });
    
    unsigned int server = node.connect(address, 2001);
    node.send(server, JOIN_MESSAGE, username);
    
    while (running)
    {
        std::string line;
        std::getline(std::cin, line);
        
        if (!line.empty())
        {
            node.send(server, CHAT_MESSAGE, line);
        }
    }
}
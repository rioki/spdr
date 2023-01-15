// chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "ChatServer.h"
#include "ChatClient.h"

int main(int argc, char* argv[])
{
    auto args_ok   = false;
    auto is_server = false;
    auto address   = std::string{};
    auto username  = std::string{};
    if (argc == 2)
    {
        if (std::strcmp(argv[1], "--server") == 0)
        {
            is_server   = true;
            args_ok     = true;
        }
    }
    if (argc == 3)
    {
        address  = argv[1];
        username = argv[2];
        args_ok     = true;
    }

    if (!args_ok)
    {
        std::cerr << "SPDR Chat Example\n"
                     "\n"
                     "Usage: \n"
                     "  To run as server: \n"
                     "    chat --server\n"
                     "\n"
                     "  To run as client:\n"
                     "    chat <address> <username> \n";
        return -1;
    }

    try
    {
        if (is_server)
        {
            auto server = chat::ChatServer{};
            server.run();
        }
        else
        {
            auto client = chat::ChatClient{address, username};
            client.run();
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unexpected exception." << std::endl;
        return -1;
    }
}

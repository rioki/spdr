// spdr
// Copyright 2011-2023 Sean Farrell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"
#include "ChatClient.h"

namespace chat
{
    ChatClient::ChatClient(const std::string& _address, const std::string& _username)
    : address(_address), username(_username)
    {
        node.on_disconnect([this] (unsigned int peer)
        {
            std::cout << "INFO: Disconnected from " << peer << "." << std::endl;
            running = false;
        });

        node.on_message<ServerMessage>([this] (auto peer, auto user, auto text)
        {
            if (user != username)
            {
                std::cout << std::setw(10) << std::left << user << ": " << text << std::endl;
            }
        });
    }

    void ChatClient::run()
    {
        std::cout << "INFO: Connecting to server " << address << " on port " << CHAT_PORT << " as " << username << "." << std::endl;
        unsigned int server = node.connect({address, CHAT_PORT});
        node.send<JoinMessage>(server, username);

        while (running)
        {
            std::string line;
            std::getline(std::cin, line);

            if (!line.empty())
            {
                node.send<ChatMessage>(server, line);
            }
        }
    }
}

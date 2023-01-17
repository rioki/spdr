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
#include "ChatServer.h"
#include "defines.h"

namespace chat
{
    ChatServer::ChatServer()
    {
        node.on_connect([] (unsigned int peer)
        {
            std::cout << "INFO: Peer " << peer << " connected." << std::endl;
        });
        node.on_disconnect([this] (unsigned int peer)
        {
            auto i = users.find(peer);
            if (i != users.end())
            {
                std::stringstream msg;
                msg << i->second << " disconnected.";
                node.broadcast<ServerMessage>("SERVER", msg.str());
                users.erase(i);
            }
            std::cout << "INFO: Peer " << peer << " disconnected." << std::endl;
        });

        node.on_message<JoinMessage>([this] (auto peer, auto name)
        {
            auto i = users.find(peer);
            if (i == users.end())
            {
                users[peer] = name;
                std::stringstream msg;
                msg << name << " joined.";
                node.broadcast<ServerMessage>("SERVER", msg.str());
            }
            else
            {
                std::cout << "WARNING: Peer " << peer << " has already sent a join message." << std::endl;
            }
        });

        node.on_message<ChatMessage>([this] (auto peer, auto text)
        {
            auto i = users.find(peer);
            if (i != users.end())
            {
                std::string name = i->second;
                node.broadcast<ServerMessage>(name, text);
                std::cout << std::setw(10) << std::left << name << ": " << text << std::endl;
            }
            else
            {
                std::cout << "WARNING: Peer " << peer << " tries to talk but has not yet joined." << std::endl;
            }
        });
    }

    void ChatServer::run()
    {
        node.listen(CHAT_PORT);
        std::cout << "INFO: Starting server on port " << CHAT_PORT << "." << std::endl;
        node.run();
    }
}

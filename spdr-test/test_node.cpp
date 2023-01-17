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

#define DEBUG_TESTS
//#define IN_DEBUGGER

#ifdef DEBUG_TESTS
    #ifdef IN_DEBUGGER
        #define TRACE(...) OutputDebugStringA(std::format(__VA_ARGS__).data())
    #else
        #define TRACE(...) std::cout << std::format(__VA_ARGS__)
    #endif
#endif

TEST(Node, echo_self_sync)
{
    bool done = false;
    constexpr auto ECHO_PORT         = 2001;
    constexpr auto ECHO_PROTOCOLL_ID = 43u;
    enum class EchoMessageId : spdr::MessageId
    {
        REQUEST,
        RESPONSE
    };

    using EchoRequest  = spdr::Message<EchoMessageId::REQUEST, std::string>;
    using EchoResponse = spdr::Message<EchoMessageId::RESPONSE, std::string>;

    auto server = spdr::Node{ECHO_PROTOCOLL_ID, std::this_thread::get_id()};
    server.on_connect([] (auto peer) {
        TRACE("Server: Peer {} connected.\n", peer);
    });
    server.on_disconnect([] (auto peer) {
        TRACE("Server: Peer {} disconnected.\n", peer);
    });
    server.on_message<EchoRequest>([&] (auto peer, auto text) {
        TRACE("Server: Recived {} from {}.\n", text, peer);
        server.send<EchoResponse>(peer, text);
    });
    server.listen(ECHO_PORT);

    auto response = std::string{};

    auto client = spdr::Node{ECHO_PROTOCOLL_ID, std::this_thread::get_id()};
    client.on_connect([] (auto peer) {
        TRACE("Client: Peer {} connected.\n", peer);
    });
    client.on_disconnect([] (auto peer) {
        TRACE("Client: Peer {} disconnected.\n", peer);
    });
    client.on_message<EchoResponse>([&] (auto peer, auto text) {
        TRACE("Client: Recived {} from {}.\n", text, peer);
        response = text;
        done = true;
    });

    auto sever_id = client.connect({"127.0.0.1", ECHO_PORT});
    auto request = std::string{"Hello SPDR!"};
    client.send<EchoRequest>(sever_id, request);

    while (!done)
    {
        c9y::sync_point();
    }

    EXPECT_EQ(request, response);
}

TEST(Node, echo_managed)
{
    constexpr auto ECHO_PORT         = 2001;
    constexpr auto ECHO_PROTOCOLL_ID = 43u;
    enum class EchoMessageId : spdr::MessageId
    {
        REQUEST,
        RESPONSE
    };

    using EchoRequest  = spdr::Message<EchoMessageId::REQUEST, std::string>;
    using EchoResponse = spdr::Message<EchoMessageId::RESPONSE, std::string>;

    auto server = spdr::Node{ECHO_PROTOCOLL_ID};
    server.on_connect([] (auto peer) {
        TRACE("Server: Peer {} connected.\n", peer);
    });
    server.on_disconnect([] (auto peer) {
        TRACE("Server: Peer {} disconnected.\n", peer);
    });
    server.on_message<EchoRequest>([&] (auto peer, auto text) {
        TRACE("Server: Recived {} from {}.\n", text, peer);
        server.send<EchoResponse>(peer, text);
    });
    server.listen(ECHO_PORT);

    auto response = std::string{};

    auto client = spdr::Node{ECHO_PROTOCOLL_ID, std::this_thread::get_id()};
    client.on_connect([] (auto peer) {
        TRACE("Client: Peer {} connected.\n", peer);
    });
    client.on_disconnect([] (auto peer) {
        TRACE("Client: Peer {} disconnected.\n", peer);
    });
    client.on_message<EchoResponse>([&] (auto peer, auto text) {
        TRACE("Client: Recived {} from {}.\n", text, peer);
        response = text;
        client.stop();
    });

    auto sever_id = client.connect({"127.0.0.1", ECHO_PORT});
    auto request = std::string{"Hello SPDR!"};
    client.send<EchoRequest>(sever_id, request);

    client.run();

    EXPECT_EQ(request, response);
}

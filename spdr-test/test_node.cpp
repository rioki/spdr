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

//#define DEBUG_TESTS
//#define IN_DEBUGGER

#ifdef DEBUG_TESTS
    #include <format>
    #ifdef IN_DEBUGGER
        #define TRACE(...) OutputDebugStringA(std::format(__VA_ARGS__).data())
    #else
        #define TRACE(...) std::cout << std::format(__VA_ARGS__)
    #endif
#else
    #define TRACE(...)
#endif

using namespace std::chrono_literals;

constexpr auto FAIL_TIMEOUT = 10s;

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

    auto start = std::chrono::steady_clock::now();
    while (!done)
    {
        c9y::sync_point();

        auto now = std::chrono::steady_clock::now();
        ASSERT_GE(FAIL_TIMEOUT, now - start);
    }

    EXPECT_EQ(request, response);
}

using fsec = std::chrono::duration<float>;

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
    auto done = std::atomic<bool>{false};

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
        client.stop();
    });

    auto sever_id = client.connect({"127.0.0.1", ECHO_PORT});
    auto request = std::string{"Hello SPDR!"};
    client.send<EchoRequest>(sever_id, request);

    // watchdog
    auto start = std::chrono::steady_clock::now();
    auto stopped = c9y::async<bool>([start, &client, &done] () {
        TRACE("Starting watchdog\n");
        while (!done)
        {
            auto now = std::chrono::steady_clock::now();
            auto dt = now - start;
            TRACE("dt: {}\n", fsec(dt).count());
            if (FAIL_TIMEOUT < now - start)
            {
                client.stop();
                TRACE("Watchdog stopped client.\n");
                return true;
            }
            std::this_thread::yield();
        }
        TRACE("Watchdog ended normally.\n");
        return false;
    });

    client.run();

    EXPECT_EQ(false, stopped.get());
    EXPECT_EQ(request, response);
}

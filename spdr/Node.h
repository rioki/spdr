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

#pragma once
#include "config.h"

#include <functional>
#include <ctime>
#include <list>
#include <map>
#include <thread>
#include <mutex>

#include "pack.h"
#include "IpAddress.h"
#include "UdpSocket.h"

namespace spdr
{
    class SPDR_EXPORT Node
    {
    public:

        Node(unsigned int id, bool threaded = true);

        ~Node();

        void listen(unsigned short port);

        unsigned int connect(const std::string& host, unsigned short port);

        void on_connect(std::function<void (unsigned int)> cb);

        void on_disconnect(std::function<void (unsigned int)> cb);

        void on_message(unsigned short id, std::function<void (unsigned int)> cb);

        template <typename T0>
        void on_message(unsigned short id, std::function<void (unsigned int, T0)> cb);

        template <typename T0, typename T1>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1)> cb);

        template <typename T0, typename T1, typename T2>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2)> cb);

        template <typename T0, typename T1, typename T2, typename T3>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3)> cb);

        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4)> cb);

        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4, T5)> cb);

        void send(unsigned int peer, unsigned int message);

        template <typename T0>
        void send(unsigned int peer, unsigned int message, T0 v0);

        template <typename T0, typename T1>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1);

        template <typename T0, typename T1, typename T2>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2);

        template <typename T0, typename T1, typename T2, typename T3>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3);

        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4);

        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5);

        void broadcast(unsigned int message);

        template <typename T0>
        void broadcast(unsigned int message, T0 v0);

        template <typename T0, typename T1>
        void broadcast(unsigned int message, T0 v0, T1 v1);

        template <typename T0, typename T1, typename T2>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2);

        template <typename T0, typename T1, typename T2, typename T3>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3);

        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4);

        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5);

        /**
         * Execute the network code in this thread.
         **/
        void run();

        void step();

    private:
        unsigned int id;

        std::function<void (unsigned int)> connect_cb;
        std::function<void (unsigned int)> disconnect_cb;
        std::map<unsigned int, std::function<void (unsigned int, std::istream&)>> messages;

        bool        threaded;
        bool        running;
        std::thread worker;

        std::recursive_mutex mutex;

        UdpSocket   socket;

        struct Peer
        {
            IpAddress address;
            clock_t   last_message_sent         = std::clock();
            clock_t   last_message_recived      = std::clock();
            unsigned int sequence_number        = 1;
            unsigned int remote_sequence_number = 0;
            unsigned int ack_field              = 0;
        };
        unsigned int next_peer_id;
        std::map<unsigned int, Peer> peers;

        struct Message
        {
            unsigned int peer;
            clock_t      time;
            unsigned int sequence_number;
            std::string  payload;
        };
        std::list<Message> sent_messages;

        void do_send(unsigned int peer, unsigned int message, std::function<void (std::ostream&)> pack_data);
        void do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data);
        bool handle_incoming();
        void handle_acks(Peer& peer, unsigned int seqnum, unsigned int lack, unsigned int acks);
        void keep_alive();
        void timeout();
        void resend_reliable();

        Node(const Node&);
        const Node& operator = (const Node&);
    };

    template <typename T0>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            unpack(is, v0);
            cb(peer, v0);
        };
    }

    template <typename T0, typename T1>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            T1 v1;
            unpack(is, v0);
            unpack(is, v1);
            cb(peer, v0, v1);
        };
    }

    template <typename T0, typename T1, typename T2>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            T1 v1;
            T2 v2;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            cb(peer, v0, v1, v2);
        };
    }

    template <typename T0, typename T1, typename T2, typename T3>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            cb(peer, v0, v1, v2, v3);
        };
    }


    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;
            T4 v4;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            unpack(is, v4);
            cb(peer, v0, v1, v2, v3, v4);
        };
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4, T5)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is)
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;
            T4 v4;
            T5 v5;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            unpack(is, v4);
            unpack(is, v5);
            cb(peer, v0, v1, v2, v3, v4, v5);
        };
    }

    template <typename T0>
    void Node::send(unsigned int peer, unsigned int message, T0 v0)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }

    template <typename T0, typename T1>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
        });
    }

    template <typename T0, typename T1, typename T2>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
            pack(os, v5);
        });
    }

    template <typename T0>
    void Node::broadcast(unsigned int message, T0 v0)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }

    template <typename T0, typename T1>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
        });
    }

    template <typename T0, typename T1, typename T2>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
        });
    }

    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
            pack(os, v5);
        });
    }
}

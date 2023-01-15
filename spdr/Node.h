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
    //! Peer Id
    using PeerId = uint32_t;

    //! Type for message ids.
    //!
    //! Use this type to define your message id enums.
    using MessageId = uint32_t;

    //! Message Type
    //!
    //! The Message class is used to define the message fileds.
    template <auto ID, typename ... PARAMS>
    struct Message
    {
        using HandleFunc = std::function<void (PeerId peer, PARAMS...)>;
        using ValueTuple = std::tuple<PARAMS...>;

        static constexpr MessageId id = static_cast<MessageId>(ID);
    };

    //! Network Node
    //!
    //! A network node is a participant in a UDP network. This can be
    //! either a server, client or peer, depending on how you configure it.
    class SPDR_EXPORT Node
    {
    public:

        Node(unsigned int id, bool threaded = true);

        ~Node();

        void listen(unsigned short port);

        //! Initiate a connection to a peer.
        PeerId connect(const std::string& host, unsigned short port);

        //! Handle a connection.
        void on_connect(std::function<void (PeerId)> cb);

        //! Handle a disconnetion.
        void on_disconnect(std::function<void (PeerId)> cb);

        //! Handle a recived message.
        template <typename MESSAGE>
        void on_message(typename MESSAGE::HandleFunc cb);

        //! Send a message to a peer.
        template <typename MESSAGE, typename ... PARAMS>
        void send(PeerId peer, PARAMS ... params);

        //! Send a message to all connected peers.
        template <typename MESSAGE, typename ... PARAMS>
        void broadcast(PARAMS ... params);


        //! Execute the network code in this thread.
        void run();

        void step();

    private:
        unsigned int id;

        std::function<void (unsigned int)> connect_cb;
        std::function<void (unsigned int)> disconnect_cb;
        std::map<MessageId, std::function<void (unsigned int, std::istream&)>> messages;

        bool        threaded = true;
        bool        running  = true;
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
        PeerId next_peer_id = 0;
        std::map<PeerId, Peer> peers;

        struct Message
        {
            PeerId       peer;
            clock_t      time;
            unsigned int sequence_number;
            std::string  payload;
        };
        std::list<Message> sent_messages;

        void do_send(PeerId peer, MesasgeId message, std::function<void (std::ostream&)> pack_data);
        void do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data);
        bool handle_incoming();
        void handle_acks(Peer& peer, unsigned int seqnum, unsigned int lack, unsigned int acks);
        void keep_alive();
        void timeout();
        void resend_reliable();

        Node(const Node&) = delete;
        const Node& operator = (const Node&) = delete;
    };

    template <typename MESSAGE>
    void Node::on_message(typename MESSAGE::HandleFunc cb)
    {
        messages[MESSAGE::id] = [=] (PeerId peer, std::istream& is)
        {
            auto values = typename MESSAGE::ValueTuple{};
            unpack(is, values);
            std::apply(std::bind_front(cb, peer), values);
        };
    }

    template <typename MESSAGE, typename ... PARAMS>
    void Node::send(PeerId peer, PARAMS ... params)
    {
        do_send(peer, MESSAGE::id, [&] (std::ostream& os) {
            (pack(os, params), ...);
        });
    }

    template <typename MESSAGE, typename ... PARAMS>
    void Node::broadcast(PARAMS ... params)
    {
        do_broadcast(MESSAGE::id, [&] (std::ostream& os) {
            (pack(os, params), ...);
        });
    }
}

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
#include <c9y/sync.h>

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

    struct BasicMessage
    {
        virtual ~BasicMessage() = default;
        virtual MessageId get_id() const = 0;
        virtual void pack(std::ostream& output) = 0;
        virtual void unpack(std::istream& input) = 0;
    };

    //! Message Type
    //!
    //! The Message class is used to define the message fileds.
    template <auto ID, typename ... PARAMS>
    struct Message : public BasicMessage
    {
        using HandleFunc = std::function<void (PeerId peer, PARAMS...)>;
        using ValueTuple = std::tuple<PARAMS...>;

        static constexpr MessageId id = static_cast<MessageId>(ID);

        ValueTuple values;

        template <typename ... CPARAMS>
        Message(CPARAMS ... params)
        : values(params...) {}

        MessageId get_id() const override
        {
            return static_cast<MessageId>(ID);
        }

        void pack(std::ostream& output) override
        {
            ::spdr::pack(output, values);
        }

        void unpack(std::istream& input) override
        {
            ::spdr::unpack(input, values);
        }
    };

    //! Network Node
    //!
    //! A network node is a participant in a UDP network. This can be
    //! either a server, client or peer, depending on how you configure it.
    class SPDR_EXPORT Node
    {
    public:
        //! Construct a node.
        Node(unsigned int id);

        //! Construct with a sync thread.
        Node(unsigned int id, std::thread::id handler_id);

        //! Destruct a node.
        ~Node();

        void listen(unsigned short port);

        //! Initiate a connection to a peer.
        PeerId connect(const IpAddress& address);

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

        //! Use this thread to handle events.
        void run();

        //! Stop the node's execution.
        void stop();

    private:
        unsigned int id;

        std::function<void (unsigned int)> connect_cb;
        std::function<void (unsigned int)> disconnect_cb;
        std::map<MessageId, std::function<void (unsigned int, std::istream&)>> handlers;

        std::atomic<bool> running  = true;
        std::jthread      worker;
        std::thread::id   handler_id;
        UdpSocket         socket;

        struct Peer
        {
            IpAddress address;
            clock_t   last_message_sent         = std::clock();
            clock_t   last_message_recived      = std::clock();
            unsigned int sequence_number        = 1;
            unsigned int remote_sequence_number = 0;
            unsigned int ack_field              = 0;
        };
        std::atomic<PeerId> next_peer_id = 0;
        std::map<PeerId, Peer> peers;

        struct Message
        {
            PeerId       peer;
            clock_t      time;
            unsigned int sequence_number;
            std::string  payload;
        };
        std::list<Message> sent_messages;

        void step();
        void do_send(PeerId peer, std::shared_ptr<BasicMessage> message);
        void do_broadcast(std::shared_ptr<BasicMessage> message);
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
        handlers[MESSAGE::id] = [this, cb] (PeerId peer, std::istream& is)
        {
            auto message = std::make_shared<MESSAGE>();
            message->unpack(is);
            c9y::sync(handler_id, [cb, peer, message] () {
                std::apply(std::bind_front(cb, peer), message->values);
            });
        };
    }

    template <typename MESSAGE, typename ... PARAMS>
    void Node::send(PeerId peer, PARAMS ... params)
    {
        auto message = std::make_shared<MESSAGE>(params...);
        c9y::sync(worker.get_id(), [this, peer, message] () mutable {
            do_send(peer, message);
        });
    }

    template <typename MESSAGE, typename ... PARAMS>
    void Node::broadcast(PARAMS ... params)
    {
        auto message = std::make_shared<MESSAGE>(params...);
        c9y::sync(worker.get_id(), [this, message] () mutable {
            do_broadcast(message);
        });
    }
}

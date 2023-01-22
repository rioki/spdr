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
#include "Node.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <climits>

#undef max

using namespace std::chrono_literals;

namespace spdr
{
    // These values probably need to made depndent on rount trip time.
    constexpr auto KEEP_ALIVE_THRESHOLD = 250ms;
    constexpr auto TIMEOUT_THRESHOLD    = 5s;
    constexpr auto RESEND_THRESHOLD     = 250ms;
    constexpr auto ACK_COUNT            = sizeof(unsigned int) * 8;

    enum SystemMessageId : MessageId
    {
        KEEP_ALIVE = std::numeric_limits<MessageId>::max()
    };

    using KeepAliveMessage = Message<SystemMessageId::KEEP_ALIVE>;

    Node::Node(unsigned int pi)
    : id(pi)
    {
        worker = std::jthread([this] (std::stop_token stop_token) {
            while (!stop_token.stop_requested())
            {
                step();
            }
        });
        handler_id = worker.get_id();
    }

    Node::Node(unsigned int pi, std::thread::id hid)
    : Node(pi)
    {
        handler_id = hid;
    }

    Node::~Node()
    {
        worker.request_stop();
        worker.join();
    }

    void Node::listen(unsigned short port)
    {
        c9y::sync(worker.get_id(), [this, port] () {
            socket.bind(port);
        });
    }

    unsigned int Node::connect(const IpAddress& address)
    {
        unsigned int pid = next_peer_id++;
        c9y::sync(worker.get_id(), [this, address, pid] () {
            peers[pid] = {address};
        });
        return pid;
    }

    void Node::on_connect(const std::function<void (PeerId)>& cb)
    {
        connect_cb = cb;
    }

    void Node::on_disconnect(const std::function<void (PeerId)>& cb)
    {
        disconnect_cb = cb;
    }

    void Node::stop()
    {
        worker.request_stop();
    }

    void Node::run()
    {
        handler_id = std::this_thread::get_id();
        auto woker_stop_token = worker.get_stop_token();
        while (!woker_stop_token.stop_requested())
        {
            c9y::sync_point();
        }
    }

    void Node::step()
    {
        assert(std::this_thread::get_id() == worker.get_id());

        c9y::sync_point();

        auto done = false;
        do
        {
            done = handle_incoming();
        }
        while (!done);

        keep_alive();
        timeout();
        resend_reliable();
    }

    void Node::do_send(PeerId peer, std::shared_ptr<BasicMessage> message)
    {
        assert(std::this_thread::get_id() == worker.get_id());

        auto i = peers.find(peer);
        if (i == peers.end())
        {
            assert(false && "No peer to send to.");
            return;
        }

        std::stringstream buff;

        pack(buff, id);
        unsigned int seqnum = i->second.sequence_number;
        i->second.sequence_number++;
        pack(buff, seqnum);

        pack(buff, message->get_id());

        pack(buff, i->second.remote_sequence_number);
        pack(buff, i->second.ack_field);

        message->pack(buff);

        std::string payload = buff.str();
        socket.send(i->second.address, payload);

        i->second.last_message_sent = std::chrono::steady_clock::now();

        if (message->get_id() != SystemMessageId::KEEP_ALIVE)
        {
            Message m = {i->first, std::chrono::steady_clock::now(), seqnum, payload};
            sent_messages.push_back(m);
        }
    }

    void Node::do_broadcast(std::shared_ptr<BasicMessage> message)
    {
        assert(std::this_thread::get_id() == worker.get_id());

        for (const auto& [id, peer] : peers)
        {
            do_send(id, message);
        }
    }

    bool Node::handle_incoming()
    {
        assert(std::this_thread::get_id() == worker.get_id());

        auto [address, data] = socket.recive();
        if (data.empty())
        {
            return true;
        }
        auto buff = std::stringstream{data};

        auto pi = unpack<unsigned int>(buff);
        if (pi != id)
        {
            return false;
        }

        auto i = std::ranges::find_if(peers, [&] (const std::pair<unsigned int, Peer>& v) {
            return v.second.address == address;
        });

        if (i == peers.end())
        {
            auto new_id = next_peer_id++;
            auto [it, inserted] = peers.try_emplace(new_id, address);
            assert(inserted);
            i = it;

            c9y::sync(handler_id, [this, peer = i->first] () {
                if (connect_cb)
                {
                    connect_cb(peer);
                }
            });
        }

        auto seqnum  = unpack<unsigned int>(buff);
        auto message = unpack<unsigned int>(buff);

        auto lack      = unpack<unsigned int>(buff);
        auto ack_field = unpack<unsigned int>(buff);

        // protect against duplicate messages
        auto recived = false;
        if (i->second.remote_sequence_number == seqnum)
        {
            recived = true;
        }
        if (i->second.remote_sequence_number > seqnum)
        {
            auto diff = i->second.remote_sequence_number - seqnum;
            if (diff < ACK_COUNT)
            {
                recived = (i->second.ack_field & (1 << (diff - 1))) != 0;
            }
            else
            {
                // message is way to old
                recived = true;
            }
        }

        handle_acks(i->second, seqnum, lack, ack_field);

        if (recived == false && message != SystemMessageId::KEEP_ALIVE)
        {
            auto mi = handlers.find(message);
            if (mi != handlers.end())
            {
                mi->second(i->first, buff);
            }
        }

        i->second.last_message_recived = std::chrono::steady_clock::now();

        return false;
    }

    void Node::handle_acks(Peer& peer, unsigned int seqnum, unsigned int lack, unsigned int acks)
    {
        assert(std::this_thread::get_id() == worker.get_id());

        // update ack info for next message
        if (peer.remote_sequence_number < seqnum)
        {
            auto diff = seqnum - peer.remote_sequence_number;
            peer.remote_sequence_number = seqnum;

            peer.ack_field = peer.ack_field << diff;
            peer.ack_field = peer.ack_field | 1 << (diff - 1);
        }
        else
        {
            auto diff = peer.remote_sequence_number - seqnum;
            if (diff < ACK_COUNT)
            {
                peer.ack_field = peer.ack_field | 1 << diff;
            }
        }

        // acknowlage messages
        for (auto i = 0u; i < ACK_COUNT; i++)
        {
            if ((acks & (1 << i)) != 0)
            {
                auto seq = lack - i;
                std::erase_if(sent_messages, [seq] (const auto& message) {
                    return message.sequence_number == seq;
                });
            }
        }
    }

    void Node::keep_alive()
    {
        assert(std::this_thread::get_id() == worker.get_id());

        auto now = std::chrono::steady_clock::now();
        std::ranges::for_each(peers, [this, now] (const auto& val) {
            if ((now - val.second.last_message_sent) > KEEP_ALIVE_THRESHOLD)
            {
                do_send(val.first, std::make_shared<KeepAliveMessage>());
            }
        });
    }

    void Node::timeout()
    {
        assert(std::this_thread::get_id() == worker.get_id());

        auto now = std::chrono::steady_clock::now();
        std::erase_if(peers, [this, now] (const auto& val) {
            if ((now - val.second.last_message_recived) > TIMEOUT_THRESHOLD) {
                if (disconnect_cb)
                {
                    disconnect_cb(val.first);
                }
                return true;
            }
            return false;
        });
    }

    void Node::resend_reliable()
    {
        assert(std::this_thread::get_id() == worker.get_id());

        auto now = std::chrono::steady_clock::now();
        std::erase_if(sent_messages, [this, now] (auto& message) {
            auto to_delete = false;

            if ((now - message.time) > RESEND_THRESHOLD)
            {
                auto i = peers.find(message.peer);
                if (i != peers.end())
                {
                    socket.send(i->second.address, message.payload);

                    message.time = now;
                    i->second.last_message_sent = now;
                }
                else
                {
                    to_delete = true;
                }
            }

            return to_delete;
        });
    }
}

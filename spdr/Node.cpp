
#include "Node.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <utility>

// TODO make this variables on Node 
#define KEEP_ALIVE_MSG        UINT_MAX
#define KEEP_ALIVE_THRESHOLD  (CLOCKS_PER_SEC / 4)
#define TIMEOUT_THRESHOLD     (2 * CLOCKS_PER_SEC)
#define RESEND_THRESHOLD      (1 * CLOCKS_PER_SEC)       

namespace spdr
{
    Node::Node(unsigned int pi, bool t)
    : id(pi), threaded(t), running(true), next_peer_id(0)
    {
        if (threaded)
        {
            worker = c9y::Thread([this] () {
                try
                {
                    run();
                }
                catch (std::exception& ex)
                {
                    std::cerr << ex.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Network worker crashed." << std::endl;
                }
            });
        }
    }
    
    Node::~Node() 
    {
        running = false;
        if (threaded)
        {            
            worker.join();
        }
    }
    
    void Node::listen(unsigned short port)
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        socket.bind(port);
    }
        
    unsigned int Node::connect(const std::string& host, unsigned short port)
    {   
        c9y::Guard<c9y::Mutex> l(mutex);
        
        unsigned int id = next_peer_id++;
        peers[id] = {IpAddress(host, port), std::clock(), std::clock()};
                
        return id;
    }
    
    void Node::on_connect(std::function<void (unsigned int)> cb)
    {
        connect_cb = cb;
    }
        
    void Node::on_disconnect(std::function<void (unsigned int)> cb)
    {
        disconnect_cb = cb;
    }
    
    void Node::on_message(unsigned short id, std::function<void (unsigned int)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            cb(peer);
        };
    }
        
    void Node::send(unsigned int peer, unsigned int message)
    {
        do_send(peer, message, [] (std::ostream&) {});
    }    
    
    void Node::broadcast(unsigned int message)
    {
        do_broadcast(message, [] (std::ostream&) {});
    }   
    
    void Node::run()
    {
        while (running)
        {
            step();
        }
    }
    
    void Node::step()
    {
        bool done = false;
        do 
        {
            done = handle_incoming();
        }
        while (!done);
                
        keep_alive();
        timeout();        
        resend_reliable();
    }   
    
    void Node::do_send(unsigned int peer, unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        auto i = peers.find(peer);
        if (i == peers.end())
        {
            throw std::invalid_argument("Invalid peer reference.");
        }
        
        std::stringstream buff;
        
        pack(buff, id);
        unsigned int seqnum = i->second.sequence_number;
        i->second.sequence_number++;
        pack(buff, seqnum);        
        
        pack(buff, message);
        
        pack(buff, i->second.remote_sequence_number);
        pack(buff, i->second.ack_field);
        
        pack_data(buff);
        
        std::string payload = buff.str();
        socket.send(i->second.address, payload);
        
        i->second.last_message_sent = std::clock();
                
        if (message != KEEP_ALIVE_MSG)
        {
            Message m = {i->first, std::clock(), seqnum, payload};
            sent_messages.push_back(m);
        }
    }
    
    void Node::do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        for (auto i = peers.begin(); i != peers.end(); i++)
        {
            do_send(i->first, message, pack_data);
        }
    }
    
    bool Node::handle_incoming()
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        IpAddress   address;
        std::string data;
        
        std::tie(address, data) = socket.recive();
        
        if (! data.empty())
        {
            std::stringstream buff(data);
            
            unsigned int pi;
            unpack(buff, pi);
            
            if (pi != id)
            {
                return false;
            }
            
            auto i = std::find_if(peers.begin(), peers.end(), [&] (const std::pair<unsigned int, Peer>& v) {
                return v.second.address == address;
            });
            
            if (i == peers.end())
            {
                unsigned int id = next_peer_id++;
                Peer peer = {address, std::clock(), std::clock(), 0, 0, 0};
                auto ii = peers.insert(std::make_pair(id, peer));
                i = ii.first;
                
                if (connect_cb)
                {
                    connect_cb(i->first);
                }
            }
                    
            unsigned int seqnum;
            unpack(buff, seqnum);        
                
            unsigned int message;
            unpack(buff, message);
            
            unsigned int lack;
            unsigned int ack_field;;
            unpack(buff, lack);
            unpack(buff, ack_field);
            
            handle_acks(i->second, seqnum, lack, ack_field);
            
            if (message != KEEP_ALIVE_MSG)
            {
                auto mi = messages.find(message);
                if (mi != messages.end())
                {
                    mi->second(i->first, buff);
                }
            }
            
            i->second.last_message_recived = std::clock();
            
            return false;
        }
        else
        {
            return true;
        }
    }
    
    void Node::handle_acks(Peer& peer, unsigned int seqnum, unsigned int lack, unsigned int acks)
    {
        static unsigned int ack_count = sizeof(unsigned int) * 8;
        
        // update ack info for next message
        if (peer.remote_sequence_number < seqnum)
        {
            unsigned int diff = seqnum - peer.remote_sequence_number;
            peer.remote_sequence_number = seqnum;
            
            peer.ack_field = peer.ack_field << diff;
            peer.ack_field = peer.ack_field | 1 << (diff - 1);
        }
        else
        {
            unsigned int diff = peer.remote_sequence_number - seqnum;
            if (diff < ack_count)
            {
                peer.ack_field = peer.ack_field | 1 << diff;
            }
        }
        
        // acknowlage messages
        for (unsigned int i = 0; i < ack_count; i++) 
        {                
            if ((acks & (1 << i)) != 0)
            {
                unsigned int seq = lack - i;
                auto i = std::find_if(sent_messages.begin(), sent_messages.end(), [&] (Message m) {
                    return m.sequence_number == seq;
                });
                if (i != sent_messages.end())
                {
                    sent_messages.erase(i);
                }
            }
        }        
    }
    
    void Node::keep_alive()
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        clock_t now = std::clock();
        
        for (auto i = peers.begin(); i != peers.end(); i++)
        {
            if ((now - i->second.last_message_sent) > KEEP_ALIVE_THRESHOLD)
            {
                send(i->first, KEEP_ALIVE_MSG);
            }
        }
    }
    
    void Node::timeout()
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        clock_t now = std::clock();
        
        auto i = peers.begin();
        while (i != peers.end())
        {
            if ((now - i->second.last_message_recived) > TIMEOUT_THRESHOLD)
            {
                if (disconnect_cb)
                {
                    disconnect_cb(i->first);
                }
                i = peers.erase(i);
            }
            else
            {
                i++;
            }
        }
    }
    
    void Node::resend_reliable()
    {
        c9y::Guard<c9y::Mutex> l(mutex);
        
        unsigned int now = std::clock();
        
        auto sm = sent_messages.begin();
        while (sm != sent_messages.end())
        {
            Message& message = *sm;
            bool deleted = false;
            
            if ((now - message.time) > RESEND_THRESHOLD) 
            {
                unsigned int peer = message.peer;
                
                auto i = peers.find(peer);
                if (i != peers.end())
                {                
                    socket.send(i->second.address, message.payload);
                
                    message.time = now; 
                    i->second.last_message_sent = now; 
                }
                else
                {
                    sm = sent_messages.erase(sm);
                    deleted = true;
                }
            }
            
            if (deleted == false)
            {
                sm++;
            }
        }
        
        std::for_each(sent_messages.begin(), sent_messages.end(), [&] (Message& message) {
            
        }); 
    }
}

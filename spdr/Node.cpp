
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
            worker = std::thread([this] () {
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
        socket.bind(port);
    }
        
    unsigned int Node::connect(const std::string& host, unsigned short port)
    {   
        std::lock_guard<std::recursive_mutex> l(mutex);
        
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
        unsigned int count = 0;
        while (running)
        {
            handle_incoming();
            
            if (count % 10)
            {
                keep_alive();
                timeout();
            }
            count++;
        }
    }
    
    void Node::do_send(unsigned int peer, unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        std::lock_guard<std::recursive_mutex> l(mutex);
        
        auto i = peers.find(peer);
        if (i == peers.end())
        {
            throw std::invalid_argument("Invalid peer reference.");
        }
        
        std::stringstream buff;
        
        pack(buff, id);
        pack(buff, message);
        
        pack_data(buff);
        
        socket.send(i->second.address, buff.str());
        
        i->second.last_message_sent = std::clock();
    }
    
    void Node::do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        std::lock_guard<std::recursive_mutex> l(mutex);
        for (auto i = peers.begin(); i != peers.end(); i++)
        {
            do_send(i->first, message, pack_data);
        }
    }
    
    void Node::handle_incoming()
    {
        std::lock_guard<std::recursive_mutex> l(mutex);
        
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
                return;
            }
            
            auto i = std::find_if(peers.begin(), peers.end(), [&] (const std::pair<unsigned int, Peer>& v) {
                return v.second.address == address;
            });
            
            if (i == peers.end())
            {
                unsigned int id = next_peer_id++;
                Peer peer = {address, std::clock(), std::clock()};
                auto ii = peers.insert(std::make_pair(id, peer));
                i = ii.first;
                
                if (connect_cb)
                {
                    connect_cb(i->first);
                }
            }
                    
            unsigned int message;
            unpack(buff, message);
            
            if (message != KEEP_ALIVE_THRESHOLD)
            {
                auto mi = messages.find(message);
                if (mi != messages.end())
                {
                    mi->second(i->first, buff);
                }
            }
            
            i->second.last_message_recived = std::clock();
        }
    }
    
    void Node::keep_alive()
    {
        std::lock_guard<std::recursive_mutex> l(mutex);
        
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
        std::lock_guard<std::recursive_mutex> l(mutex);
        
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
}


#include "Node.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <c9y/c9y.h>

#include "Peer.h"

namespace spdr
{
    enum SystemMessageIds
    {
        MSG_MISSMATCH  = 254,
        MSG_KEEP_ALIVE = 255
    };

    Node::Node(unsigned char pi, unsigned char pv)
    : id(pi), version(pv), loop(NULL), socket(NULL), worker(NULL)
    {
        loop = new c9y::EventLoop;    
        socket = new c9y::UdpSocket(*loop);        
    }
    
    Node::~Node()
    {        
        stop(); // this can happen when node is running multithreaded
    
        assert(worker == NULL);
    
        delete socket;
        delete loop;
    }
    
    unsigned char Node::get_id() const
    {
        return id;
    }
    
    unsigned char Node::get_version() const
    {
        return version;
    }
              
    void Node::listen(unsigned short port, std::function<void (Peer*)> ccb, std::function<void (Peer*)> dcb)
    {
        connect_cb    = ccb;
        disconnect_cb = dcb;
    
        socket->bind(port);
        socket->receive([this] (const c9y::IpAddress& address, const char* data, size_t len) 
        {
            handle_message(address, data, len);
        });
    }
        
    void Node::connect(const std::string& address, unsigned short port, std::function<void (Peer*)> ccb, std::function<void (Peer*)> dcb)
    {
        connect_cb    = ccb;
        disconnect_cb = dcb;
        
        Peer* peer = new Peer(address, port);
        peers.push_back(peer);
        
        // first msg or keep alive will init the actual connection
        socket->receive([this] (const c9y::IpAddress& address, const char* data, size_t len) 
        {
            handle_message(address, data, len);
        });
        
        connect_cb(peer);
    }
    
    void Node::on_message(unsigned short id, std::function<void (Peer*)> cb)
    {
        messages[id] = [=] (Peer* peer, std::istream& is) 
        {
            cb(peer);
        };
    }
        
    void Node::send(Peer* peer, unsigned char message)
    {
        generic_send(peer, message, [] (std::ostream&) {});
    }
    
    void Node::generic_send(Peer* peer, unsigned char message, std::function<void (std::ostream&)> pack_data)
    {
        std::string data = build_message(message, pack_data);
        
        c9y::IpAddress adr(peer->get_address(), peer->get_port());
        socket->send(adr, data.c_str(), data.size());
    }
    
    void Node::broadcast(unsigned char message)
    {
        generic_broadcast(message, [] (std::ostream&) {});
    }
    
    void Node::generic_broadcast(unsigned char message, std::function<void (std::ostream&)> pack_data)
    {
        std::string data = build_message(message, pack_data);
        
        for (Peer* peer : peers)
        {
            c9y::IpAddress adr(peer->get_address(), peer->get_port());
            socket->send(adr, data.c_str(), data.size());
        }
    }
    
    void Node::run()
    {
        loop->run();
    }
    
    void Node::start()
    {
        assert(worker == NULL);
        
        worker = new c9y::Thread([this] () {
            run();
        });
    }
        
    void Node::stop()
    {
        loop->stop();
        if (worker)
        {
            worker->join();
            delete worker;
            worker = NULL;
        }
    }
    
    void Node::handle_message(const c9y::IpAddress& address, const char* data, size_t len)
    {
        auto pi = std::find_if(peers.begin(), peers.end(), [&] (Peer* peer) {
                return peer->get_address() == address.get_ip() &&
                       peer->get_port() == address.get_post(); // FIXME c9y name?!
        });
        
        std::stringstream buff(std::string(data, len));
        
        unsigned char remote_id = 0;
        unsigned char remote_version = 0;
        unpack(buff, remote_id);
        unpack(buff, remote_version);
        
        if (id != remote_id || version != remote_version) 
        {            
            return; // drop invalid formates message
        }
        
        Peer* peer = NULL;
        if (pi == peers.end()) // new connection
        {
            peer = new Peer(address.get_ip(), address.get_post());
            peers.push_back(peer);
            connect_cb(peer);            
        }
        else
        {
            peer = *pi;
        }
                        
        unsigned char message = 0;
        unpack(buff, message);
        
        // ack crap
        
        auto mi = messages.find(message);
        if (mi != messages.end())
        {
            mi->second(peer, buff);
        }
        else
        {
            // error handling?
        }
    }
    
    std::string Node::build_message(unsigned char message, std::function<void (std::ostream&)> pack_data)
    {
        std::stringstream buff;
        pack(buff, id);
        pack(buff, version);
        pack(buff, message);
        // pack ack stuff
        pack_data(buff);
        
        return buff.str();
    }
    

}

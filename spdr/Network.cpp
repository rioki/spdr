// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "Network.h"

#include <sstream>
#include <algorithm>
#include <ctime>
#include <typeinfo>

#include "KeepAliveMessage.h"

#define DEBUG
#include "debug.h" 

namespace spdr
{
    unsigned int get_time()
    {
        return std::clock() / (CLOCKS_PER_SEC / 1000);
    }  
    
    struct info_with_address
    {
        const Address& address;
        
        info_with_address(const Address& a)
        : address(a) {}
        
        bool operator () (PeerInfo* info)
        {
            return info->get_address() == address;
        }
    };
    
    Network::Network(unsigned int pi)
    : protocol_id(pi), socket(), 
      worker(sigc::mem_fun(this, &Network::worker_main)) 
    {
        register_system_messages();
        running = true;
        worker.start();
    }
    
    Network::Network(unsigned int pi, unsigned int po)
    : protocol_id(pi), socket(po),
      worker(sigc::mem_fun(this, &Network::worker_main)) 
    {
        register_system_messages();
        running = true;
        worker.start();
    }
    
    Network::~Network() 
    {
        try
        {
            running = false;
            worker.join();                        
            
            for (PeerInfo* peer : peers)
            {
                delete peer;
            }
            peers.clear();
        }
        catch (std::exception& ex)
        {
            // weep silently
        }
    }

    unsigned int Network::get_protocol_id() const 
    {
        return protocol_id;
    }
    
    PeerInfo Network::connect(Address address)
    {
        // NOTE: 
        // The connection is initiated by a keep alive. Here we just create 
        // a PeerInfo and the normal keep alive will initiate the comunication.
        
        TRACE("Connecting to %d.%d.%d.%d:%d.", address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port());
        
        PeerInfo* peer = get_info(address);
        return *peer;
    }
    
    void Network::disconnect(Address address)
    {
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        
        TRACE("Disconnecting from %d.%d.%d.%d:%d.", address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port());
        
        PeerInfo* peer = get_info(address, false);
        if (peer != NULL && peer->connected)
        {
            peer->connected = false;
            peer->disconnecting = true;
            disconnect_signal.emit(*peer);
        }
    }
    
    sigc::signal<void, PeerInfo>& Network::get_connect_signal()
    {
        return connect_signal;
    }
    
    sigc::signal<void, PeerInfo>& Network::get_disconnect_signal()
    {
        return disconnect_signal;
    }
    
    void Network::send(const PeerInfo& info, Message* message)
    {
        c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
        send_queue.push(std::make_tuple(info, message));
    }
    
    sigc::signal<void, PeerInfo, Message&>& Network::get_message_signal()
    {
        return message_signal;
    }
    
    void Network::register_system_messages()
    {
        register_message<KeepAliveMessage>(KEEP_ALIVE_MESSAGE);
    }
    
    void Network::worker_main()
    {        
        while (running)
        {
            do_send();
            do_recive();
            do_keep_alive();
            do_timeout();
        }
    }
    
    void Network::do_send()
    {        
        PeerInfo info;
        Message* message;
        {
            c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
            if (send_queue.empty())
            {
                return;            
            }
            std::tie(info, message) = send_queue.front();
            send_queue.pop();
        }
        
        {
            c9y::Lock<c9y::Mutex> lock(peers_mutex);
                
            PeerInfo* peer = get_info(info.get_address());
            peer->last_message_sent = get_time();            
        }
        
        TRACE("Sending Message %d to %d.%d.%d.%d:%d.", message->get_id(), info.get_address().get_a(), info.get_address().get_b(), info.get_address().get_c(), info.get_address().get_d(), info.get_address().get_port());
        
        // REVIEW: Should we check here if the node info still is valid?
        std::stringstream buff;
        pack(buff, protocol_id);
        pack(buff, message->get_id());
        message->pack(buff);
        
        socket.send(info.get_address(), buff.str());
    }    
    
    void Network::do_recive()
    {
        Address address;
        std::string data;        
        std::tie(address, data) = socket.recive();
        
        if (data.empty())
        {
            return;
        }
        
        std::stringstream buff(data);
        
        unsigned int in_pid = 0;
        unsigned int message_id = 0;
        
        unpack(buff, in_pid);                
        if (in_pid != protocol_id)
        {
            return;
        }
        
        PeerInfo* peer = get_info(address);
        if (peer->connected == false && peer->disconnecting == false)
        {
            TRACE("Connected to %d.%d.%d.%d:%d.", address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port());            
            peer->connected = true;
            connect_signal.emit(*peer);
        }        
        peer->last_message_recived = get_time();
        
        unpack(buff, message_id);
        Message* message = create_message(message_id);
        if (message == NULL)
        {
            return;
        }
        
        message->unpack(buff);
        
        TRACE("Recived Message %d to %d.%d.%d.%d:%d.", message->get_id(), address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port());
        
        if (message_id != KEEP_ALIVE_MESSAGE)
        {            
            message_signal.emit(*peer, *message);
        }
        
        delete message;
    }
    
    void Network::do_keep_alive()
    {
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        unsigned int now = get_time();

        std::list<PeerInfo*>::iterator iter = peers.begin();
        while (iter != peers.end())
        {
            if ((*iter)->connected && (now - (*iter)->last_message_sent) > 250)
            {
                send(**iter, new KeepAliveMessage);
                (*iter)->last_message_sent = now;
            }
            ++iter;
        }
    }
    
    void Network::do_timeout()
    {
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        unsigned int now = get_time();
        
        std::list<PeerInfo*>::iterator iter = peers.begin();
        while (iter != peers.end())
        {
            unsigned int timeout = (*iter)->disconnecting ? 4000 : 2000;            
            
            if ((now - (*iter)->last_message_recived) > timeout)
            {
                TRACE("Disconnected from %d.%d.%d.%d:%d.", (*iter)->address.get_a(), (*iter)->address.get_b(), (*iter)->address.get_c(), (*iter)->address.get_d(), (*iter)->address.get_port());
                
                // if the connected flag is false the diconnect_signal was already emitted
                if ((*iter)->connected == false) 
                {
                    disconnect_signal.emit(**iter);
                }
                delete *iter;
                iter = peers.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    
    PeerInfo* Network::get_info(const Address& address, bool create) 
    {
        std::list<PeerInfo*>::iterator iter;
        iter = std::find_if(peers.begin(), peers.end(), info_with_address(address));
        if (iter != peers.end())
        {
            return *iter;
        }
        else 
        {
            if (create)
            {                
                PeerInfo* info = new PeerInfo(address, get_time());        
                info->last_message_recived = get_time();
                peers.push_back(info);                
                return info;
            }
            else
            {
                return NULL;
            }
        }
    }
    
    Message* Network::create_message(unsigned int id) 
    {
        c9y::Lock<c9y::Mutex> lock(message_map_mutex);
        std::map<unsigned int, MessageCreator>::iterator iter;
        iter = message_map.find(id);
        if (iter != message_map.end())
        {
            return iter->second();
        }
        else
        {
            return NULL;    
        }
    }
}

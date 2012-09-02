// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "Network.h"

#include <sstream>
#include <algorithm>
#include <ctime>
#include <typeinfo>

#include "debug.h" 
#include "KeepAliveMessage.h"

namespace spdr
{    
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
    
    struct message_with_sequence_number
    {
        unsigned int sequence_number;
        
        message_with_sequence_number(unsigned int sn)
        : sequence_number(sn) {}
        
        bool operator () (const std::tuple<PeerInfo, Message*>& obj)
        {
            return std::get<1>(obj)->get_sequence_number() == sequence_number;
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
            //disconnect_signal.emit(*peer);
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
            for (unsigned int i = 0; i < 10; i++)
            {
                do_send();
                do_recive();
            }
            do_keep_alive();
            do_timeout();
            do_reliable_messages();
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
        
        // TODO: ack
        unsigned int last_ack = 0;
        unsigned int ack_field = 0;
        
        {
            c9y::Lock<c9y::Mutex> lock(peers_mutex);
                
            PeerInfo* peer = get_info(info.get_address(), false);
            if (peer == NULL)
            {
                delete message;
                return; // we are not connected to the peer anyore
            }
            peer->last_message_sent = std::clock(); 
            message->sequence_number = peer->last_sequence_number++;
            last_ack = peer->remote_sequence_number;
            ack_field = peer->ack_field;
        }
        
        TRACE("Sending Message type %d number %d to %d.%d.%d.%d:%d.", message->get_id(), message->sequence_number, info.get_address().get_a(), info.get_address().get_b(), info.get_address().get_c(), info.get_address().get_d(), info.get_address().get_port());
        
        // REVIEW: Should we check here if the node info still is valid?
        std::stringstream buff;
        pack(buff, protocol_id);
        pack(buff, message->get_id());
        pack(buff, message->sequence_number);
        pack(buff, last_ack);
        pack(buff, ack_field);
        message->pack(buff);
        
        socket.send(info.get_address(), buff.str());
        message->sent_time = std::clock();
        
        {
            c9y::Lock<c9y::Mutex> lock(sent_messages_mutex);
            sent_messages.push_back(std::make_tuple(info, message));
        }
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
        peer->last_message_recived = std::clock();
        
        unpack(buff, message_id);
        Message* message = create_message(message_id);
        if (message == NULL)
        {
            return;
        }
        
        unsigned int last_ack = 0;
        unsigned int ack_field = 0;
        
        unpack(buff, message->sequence_number);
        unpack(buff, last_ack);
        unpack(buff, ack_field);
        
        message->unpack(buff);
        
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        handle_incomming_acks(get_info(address), message->sequence_number, last_ack, ack_field);
        
        TRACE("Recived Message type %d number %d from %d.%d.%d.%d:%d.", message->get_id(), message->sequence_number, address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port());
        
        if (message_id != KEEP_ALIVE_MESSAGE)
        {            
            message_signal.emit(*peer, *message);
        }        
        
        delete message;
    }
    
    void Network::handle_incomming_acks(PeerInfo* peer, unsigned int sequence_number, unsigned int last_ack, unsigned int ack_field)
    {
        static unsigned int ack_count = sizeof(unsigned int) * 8;
        
        // update ack info for next message
        if (peer->remote_sequence_number < sequence_number)
        {
            peer->remote_sequence_number = sequence_number;
            unsigned int diff = sequence_number - peer->remote_sequence_number;
            peer->ack_field = peer->ack_field << diff;
        }
        else
        {
            unsigned int diff = peer->remote_sequence_number - sequence_number;
            if (diff < sizeof(unsigned int) * 8)
            {
                peer->ack_field = peer->ack_field | 1 << diff;
            }
        }
        
        {
            c9y::Lock<c9y::Mutex> lock(sent_messages_mutex);
            for (unsigned int i = 0; i < ack_count; i++) 
            {                
                if (i < sent_messages.size() && (ack_field & (1 << i)) != 0)
                {
                    // TODO handle id rollover
                    unsigned int seq = last_ack - i;
                    auto iter = std::find_if(sent_messages.begin(), sent_messages.end(), message_with_sequence_number(seq));
                    if (iter != sent_messages.end())
                    {
                        // message was acknowlaged, thus remove the message
                        Message* message = std::get<1>(*iter);
                        delete message;
                        sent_messages.erase(iter);
                    }
                }
            }
        }
    }
    
    void Network::do_keep_alive()
    {
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        unsigned int now = std::clock();

        std::list<PeerInfo*>::iterator iter = peers.begin();
        while (iter != peers.end())
        {
            if ((*iter)->connected && (now - (*iter)->last_message_sent) > (CLOCKS_PER_SEC / 4))
            {
                send(**iter, KeepAliveMessage());
                (*iter)->last_message_sent = now;
            }
            ++iter;
        }
    }
    
    void Network::do_timeout()
    {
        c9y::Lock<c9y::Mutex> lock(peers_mutex);
        unsigned int now = std::clock();
        
        std::list<PeerInfo*>::iterator iter = peers.begin();
        while (iter != peers.end())
        {
            unsigned int timeout = (*iter)->disconnecting ? (3 * CLOCKS_PER_SEC) : (2 * CLOCKS_PER_SEC);
            
            if ((now - (*iter)->last_message_recived) > timeout)
            {
                TRACE("Disconnected from %d.%d.%d.%d:%d.", (*iter)->address.get_a(), (*iter)->address.get_b(), (*iter)->address.get_c(), (*iter)->address.get_d(), (*iter)->address.get_port());
                
                disconnect_signal.emit(**iter);
                delete *iter;
                iter = peers.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    
    void Network::do_reliable_messages()
    {
        c9y::Lock<c9y::Mutex> lock(sent_messages_mutex);
        unsigned int now = std::clock();
        
        auto iter = sent_messages.begin();
        while (iter != sent_messages.end())
        {
            unsigned int timeout = 1 * CLOCKS_PER_SEC;
            
            PeerInfo peer = std::get<0>(*iter);
            Message* message = std::get<1>(*iter);
            if ((now - message->sent_time) > timeout)
            {
                iter = sent_messages.erase(iter);
                if (message->is_reliable())
                {
                    TRACE("Message type %d number %d to %d.%d.%d.%d:%d timed out, resending.", message->get_id(), message->sequence_number, peer.address.get_a(), peer.address.get_b(), peer.address.get_c(), peer.address.get_d(), peer.address.get_port());
                    
                    c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
                    send_queue.push(std::make_tuple(peer, message));
                }
                else
                {
                    TRACE("Message type %d number %d to %d.%d.%d.%d:%d timed out.", message->get_id(), message->sequence_number, peer.address.get_a(), peer.address.get_b(), peer.address.get_c(), peer.address.get_d(), peer.address.get_port());
                    delete message;
                }                
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
                PeerInfo* info = new PeerInfo(address, std::clock());        
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

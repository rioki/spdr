
#include "Node.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <climits>

#include <c9y/c9y.h>

#include "Peer.h"

#define KEEP_ALIVE_MSG        UINT_MAX
#define KEEP_ALIVE_THRESHOLD  (CLOCKS_PER_SEC / 4)
#define TIMEOUT_THRESHOLD     (2 * CLOCKS_PER_SEC)
#define RESEND_THRESHOLD      (1 * CLOCKS_PER_SEC)       

#if DEBUG
#define TRACE(MSG) std::cout << std::clock() << " " <<  __FILE__ << "(" << __LINE__ << "): " << MSG << std::endl
#else
#define TRACE(MSG)
#endif


namespace spdr
{
    Node::Node(unsigned int pi, unsigned int pv)
    : id(pi), version(pv), loop(NULL), socket(NULL), worker(NULL)
    {
        loop = new c9y::EventLoop;    
        socket = new c9y::UdpSocket(*loop);    

        timer = new c9y::Timer(*loop, 100, [this] () {
            do_keep_alive();
            do_timeout();
            do_reliable_messages();
        });
    }
    
    Node::~Node()
    {        
        stop(); // this can happen when node is running multithreaded
    
        assert(worker == NULL);
    
        delete socket;
        delete loop;
    }
    
    unsigned int Node::get_id() const
    {
        return id;
    }
    
    unsigned int Node::get_version() const
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
        
        Peer* peer = new Peer(address, port, std::clock());
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
        
    void Node::send(Peer* peer, unsigned int message)
    {
        do_send(peer, message, [] (std::ostream&) {});
    }    
    
    void Node::broadcast(unsigned int message)
    {
        do_broadcast(message, [] (std::ostream&) {});
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
        
        unsigned int remote_id = 0;
        unsigned int remote_version = 0;
        
        unpack(buff, remote_id);
        unpack(buff, remote_version);
        
        if (id != remote_id || version != remote_version) 
        {            
            return; // drop invalid formates message
        }
        
        unsigned int last_ack = 0;
        unsigned int ack_field = 0;
        unpack(buff, last_ack);
        unpack(buff, ack_field);
        
        
        Peer* peer = NULL;
        if (pi == peers.end()) // new connection
        {
            peer = new Peer(address.get_ip(), address.get_post(), std::clock());
            peers.push_back(peer);
            connect_cb(peer);            
        }
        else
        {
            peer = *pi;
        }
        
        peer->last_message_recived = std::clock();

        unsigned int message = 0;
        unsigned int sequence_number = 0; 
        unpack(buff, message);
        unpack(buff, sequence_number);
        
        handle_incomming_acks(peer, sequence_number, last_ack, ack_field);
        
        TRACE("Message " << message << " / " << sequence_number << " recived.");
        
        if (message != KEEP_ALIVE_MSG)
        {        
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
    }
    
    void Node::handle_incomming_acks(Peer* peer, unsigned int sequence_number, unsigned int last_ack, unsigned int ack_field)
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
            if (diff < ack_count)
            {
                peer->ack_field = peer->ack_field | 1 << diff;
            }
        }
        
        for (unsigned int i = 0; i < ack_count; i++) 
        {                
            if ((ack_field & (1 << i)) != 0)
            {
                // TODO handle id rollover
                unsigned int seq = last_ack - i;
                auto iter = std::find_if(sent_messages.begin(), sent_messages.end(), [&] (Message* m) {
                    return m->number == seq;
                });
                if (iter != sent_messages.end())
                {
                    TRACE("Message " << seq << " acknowledged.");
                    // message was acknowlaged, thus remove the message
                    Message* message = *iter;
                    delete message;
                    sent_messages.erase(iter);
                }
            }
        }        
    }
        
    void Node::do_send(Peer* peer, unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        clock_t now = std::clock();
        
        unsigned int sequence_number = peer->last_sequence_number++;
        unsigned int last_ack = peer->remote_sequence_number;
        unsigned int ack_field = peer->ack_field;
        
        TRACE("Sending message " << message << " / " << sequence_number << " .");
        
        std::stringstream buff;
        pack(buff, id);
        pack(buff, version);                
        pack(buff, last_ack);
        pack(buff, ack_field);        
        pack(buff, message);  
        pack(buff, sequence_number);        
        pack_data(buff);
        
        c9y::IpAddress adr(peer->get_address(), peer->get_port());
        
        std::string data = buff.str();
        socket->send(adr, data.c_str(), data.size());
        peer->last_message_sent = now; 
        
        if (message != KEEP_ALIVE_MSG)
        {
            // NOTE: the values could be filled directly
            Message* msg = new Message;
            msg->peer = peer;
            msg->time = now;
            msg->number = sequence_number;
            msg->data = data;
            sent_messages.push_back(msg);
        }
    }
    
    void Node::do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data)
    {
        for (Peer* peer : peers)
        {
            // OK this is innefficient, but I currently don't have a 
            // efficient was that also handles acks proeprly
            do_send(peer, message, pack_data);
        }
    }
    
    void Node::do_keep_alive()
    {
        unsigned int now = std::clock();
        
        for (Peer* peer : peers)
        {
            if ((now - peer->last_message_sent) > KEEP_ALIVE_THRESHOLD)
            {                                
                send(peer, KEEP_ALIVE_MSG);
                peer->last_message_sent = now;
            }
        }
    }
    
    void Node::do_timeout()
    {
        unsigned int now = std::clock();
        
        auto i = std::remove_if(peers.begin(), peers.end(), [&] (Peer* peer) {
            return (now - peer->last_message_recived) > TIMEOUT_THRESHOLD;
        });
        
        std::for_each(i, peers.end(), [&] (Peer* peer) {
            auto j = std::remove_if(sent_messages.begin(), sent_messages.end(), [&] (Message* message) {
                return message->peer == peer;
            });
            TRACE("Erase " << std::distance(j, sent_messages.end()) << " pending messages.");
            sent_messages.erase(j, sent_messages.end());            
        });
        
        std::vector<Peer*> tmp(i, peers.end());                
        peers.erase(i, peers.end());
        
        for (Peer* peer : tmp)
        {
            disconnect_cb(peer);
            delete peer;
        }
    }
    
    void Node::do_reliable_messages()
    {
        unsigned int now = std::clock();
        
        std::for_each(sent_messages.begin(), sent_messages.end(), [&] (Message* message) {
            if ((now - message->time) > RESEND_THRESHOLD) 
            {
                Peer* peer = message->peer;
                
                c9y::IpAddress adr(peer->get_address(), peer->get_port());
                socket->send(adr, message->data.c_str(), message->data.size());
                
                TRACE("Resending message " << message->number << " .");
                
                message->time = now; 
                peer->last_message_sent = now; 
            }
        }); 
    }
}

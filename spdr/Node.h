
#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <functional>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <iosfwd>

#include "pack.h"
#include "Peer.h"

namespace c9y 
{
    class IpAddress;
    class UdpSocket;
    class Interval;
}

namespace spdr
{
    class Peer;

    class Node
    {
    public:
    
        /**
         * Create a Node
         *
         * @param id the protocol id
         * @param version the protocol version
         **/
        Node(unsigned int id, unsigned int version);
        
        ~Node();
        
        unsigned int get_id() const;   
        
        unsigned int get_version() const;
        
        void listen(unsigned short port);
        
        Peer* connect(const std::string& address, unsigned short port);
        
        void on_connect(std::function<void (Peer*)> cb);
        
        void on_disconnect(std::function<void (Peer*)> cb);
        
        void on_message(unsigned short id, std::function<void (Peer*)> cb);
        
        template <typename T0>
        void on_message(unsigned short id, std::function<void (Peer*, T0)> cb);
        
        template <typename T0, typename T1>
        void on_message(unsigned short id, std::function<void (Peer*, T0, T1)> cb);
        
        void send(Peer* peer, unsigned int message);
        
        template <typename T0>
        void send(Peer* peer, unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void send(Peer* peer, unsigned int message, T0 v0, T1 v1);

        void broadcast(unsigned int message);
        
        template <typename T0>
        void broadcast(unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void broadcast(unsigned int message, T0 v0, T1 v1);               
        
        /**
         * Execute the network code in this thread.         
         **/
        void run();
        
        void stop();
        
    
    private:
        unsigned int id;
        unsigned int version;
        c9y::UdpSocket* socket;        
        c9y::Interval*  interval;
        std::function<void (Peer*)> connect_cb;
        std::function<void (Peer*)> disconnect_cb;        
        
        std::vector<Peer*> peers;
        std::map<unsigned int, std::function<void (Peer*, std::istream&)>> messages;
        
        struct Message
        {
            Peer*        peer;
            clock_t      time;
            unsigned int number;
            std::string  data;
        };
        std::list<Message*> sent_messages; // messages without an ack
        
        void handle_message(const c9y::IpAddress& address, const char* data, size_t len);        
        void handle_incomming_acks(Peer* peer, unsigned int sequence_number, unsigned int last_ack, unsigned int ack_field);
        
        void do_send(Peer* peer, unsigned int message, std::function<void (std::ostream&)> pack_data);
        void do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data);
        
        void do_keep_alive();
        void do_timeout();
        void do_reliable_messages();
        
        Node(const Node&);
        const Node& operator = (const Node&);
    };
    
    template <typename T0>
    void Node::on_message(unsigned short id, std::function<void (Peer*, T0)> cb)
    {
        messages[id] = [=] (Peer* peer, std::istream& is) 
        {
            T0 v0;
            unpack(is, v0);
            cb(peer, v0);
        };
    }
    
    template <typename T0, typename T1>
    void Node::on_message(unsigned short id, std::function<void (Peer*, T0, T1)> cb)
    {
        messages[id] = [=] (Peer* peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            unpack(is, v0);
            unpack(is, v1);
            cb(peer, v0, v1);
        };
    }
    
    template <typename T0>
    void Node::send(Peer* peer, unsigned int message, T0 v0)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }
    
    template <typename T0, typename T1>
    void Node::send(Peer* peer, unsigned int message, T0 v0, T1 v1)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
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
}

#endif

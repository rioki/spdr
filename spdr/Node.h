
#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <functional>
#include <ctime>
#include <list>
#include <map>

#include "pack.h"
#include "IpAddress.h"
#include "UdpSocket.h"

#include <c9y/c9y.h>

namespace spdr
{
    class Node
    {
    public:
           
        Node(unsigned int id, bool threaded = true);
        
        ~Node();
        
        void listen(unsigned short port);
        
        unsigned int connect(const std::string& host, unsigned short port);
        
        void on_connect(std::function<void (unsigned int)> cb);
        
        void on_disconnect(std::function<void (unsigned int)> cb);
        
        void on_message(unsigned short id, std::function<void (unsigned int)> cb);
        
        template <typename T0>
        void on_message(unsigned short id, std::function<void (unsigned int, T0)> cb);
        
        template <typename T0, typename T1>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1)> cb);
        
        template <typename T0, typename T1, typename T2>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2)> cb);
        
        template <typename T0, typename T1, typename T2, typename T3>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3)> cb);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4)> cb);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4, T5)> cb);
        
        void send(unsigned int peer, unsigned int message);
        
        template <typename T0>
        void send(unsigned int peer, unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1);
        
        template <typename T0, typename T1, typename T2>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2);
        
        template <typename T0, typename T1, typename T2, typename T3>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5);

        void broadcast(unsigned int message);
        
        template <typename T0>
        void broadcast(unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void broadcast(unsigned int message, T0 v0, T1 v1);             
        
        template <typename T0, typename T1, typename T2>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2);
        
        template <typename T0, typename T1, typename T2, typename T3>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4);
        
        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        void broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5);
        
        /**
         * Execute the network code in this thread.         
         **/
        void run();     

        void step();
    
    private:
        unsigned int id;
        
        std::function<void (unsigned int)> connect_cb;
        std::function<void (unsigned int)> disconnect_cb;        
        std::map<unsigned int, std::function<void (unsigned int, std::istream&)>> messages;
        
        bool        threaded;
        bool        running;
        c9y::Thread worker;
        
        c9y::Mutex  mutex;
        
        UdpSocket   socket;
        
        struct Peer
        {
            IpAddress address;
            clock_t   last_message_sent;
            clock_t   last_message_recived;
            unsigned int sequence_number;
            unsigned int remote_sequence_number;
            unsigned int ack_field;
        };
        unsigned int next_peer_id;
        std::map<unsigned int, Peer> peers;
        
        struct Message
        {
            unsigned int peer;
            clock_t      time;
            unsigned int sequence_number;
            std::string  payload;
        };
        std::list<Message> sent_messages;
        
        void do_send(unsigned int peer, unsigned int message, std::function<void (std::ostream&)> pack_data);
        void do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data);
        bool handle_incoming();
        void handle_acks(Peer& peer, unsigned int seqnum, unsigned int lack, unsigned int acks);
        void keep_alive();
        void timeout();
        void resend_reliable();
        
        Node(const Node&);
        const Node& operator = (const Node&);
    };
    
    template <typename T0>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            unpack(is, v0);
            cb(peer, v0);
        };
    }
    
    template <typename T0, typename T1>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            unpack(is, v0);
            unpack(is, v1);
            cb(peer, v0, v1);
        };
    }
    
    template <typename T0, typename T1, typename T2>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            T2 v2;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            cb(peer, v0, v1, v2);
        };
    }
    
    template <typename T0, typename T1, typename T2, typename T3>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;            
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            cb(peer, v0, v1, v2, v3);
        };
    }
    
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;
            T4 v4;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            unpack(is, v4);
            cb(peer, v0, v1, v2, v3, v4);
        };
    }
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::on_message(unsigned short id, std::function<void (unsigned int, T0, T1, T2, T3, T4, T5)> cb)
    {
        messages[id] = [=] (unsigned int peer, std::istream& is) 
        {
            T0 v0;
            T1 v1;
            T2 v2;
            T3 v3;
            T4 v4;
            T5 v5;
            unpack(is, v0);
            unpack(is, v1);
            unpack(is, v2);
            unpack(is, v3);
            unpack(is, v4);
            unpack(is, v5);
            cb(peer, v0, v1, v2, v3, v4, v5);
        };
    }
    
    template <typename T0>
    void Node::send(unsigned int peer, unsigned int message, T0 v0)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }
    
    template <typename T0, typename T1>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
        });
    }
    
    template <typename T0, typename T1, typename T2>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::send(unsigned int peer, unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5)
    {
        do_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
            pack(os, v5);
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
    
    template <typename T0, typename T1, typename T2>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
        });
    }
    
    template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void Node::broadcast(unsigned int message, T0 v0, T1 v1, T2 v2, T3 v3, T4 v4, T5 v5)
    {
        do_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
            pack(os, v2);
            pack(os, v3);
            pack(os, v4);
            pack(os, v5);
        });
    }
}

#endif

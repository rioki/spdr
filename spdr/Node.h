
#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <functional>
#include <ctime>
#include <map>
#include <mutex>
#include <thread>

#include "pack.h"
#include "IpAddress.h"
#include "UdpSocket.h"

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
        
        void send(unsigned int peer, unsigned int message);
        
        template <typename T0>
        void send(unsigned int peer, unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void send(unsigned int peer, unsigned int message, T0 v0, T1 v1);

        void broadcast(unsigned int message);
        
        template <typename T0>
        void broadcast(unsigned int message, T0 v0);
        
        template <typename T0, typename T1>
        void broadcast(unsigned int message, T0 v0, T1 v1);               
        
        /**
         * Execute the network code in this thread.         
         **/
        void run();        
    
    private:
        unsigned int id;
        
        std::function<void (unsigned int)> connect_cb;
        std::function<void (unsigned int)> disconnect_cb;        
        std::map<unsigned int, std::function<void (unsigned int, std::istream&)>> messages;
        
        bool        threaded;
        bool        running;
        std::thread worker;
        
        std::recursive_mutex mutex;
        
        UdpSocket   socket;
        
        struct Peer
        {
            IpAddress address;
            clock_t   last_message_sent;
            clock_t   last_message_recived;
        };
        unsigned int next_peer_id;
        std::map<unsigned int, Peer> peers;
        
        void do_send(unsigned int peer, unsigned int message, std::function<void (std::ostream&)> pack_data);
        void do_broadcast(unsigned int message, std::function<void (std::ostream&)> pack_data);
        void handle_incoming();
        void keep_alive();
        void timeout();
        
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

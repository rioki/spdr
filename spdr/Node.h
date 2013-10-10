
#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <functional>
#include <string>
#include <vector>
#include <map>
#include <iosfwd>

#include "pack.h"
#include "Peer.h"

namespace c9y 
{
    class IpAddress;
    class UdpSocket;
    class Timer;
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
        Node(unsigned char id, unsigned char version);
        
        ~Node();
        
        unsigned char get_id() const;   
        
        unsigned char get_version() const;
        
        void listen(unsigned short port, std::function<void (Peer*)> connect_cb, std::function<void (Peer*)> disconnect_cb);
        
        void connect(const std::string& address, unsigned short port, std::function<void (Peer*)> connect_cb, std::function<void (Peer*)> disconnect_cb);
        
        void on_message(unsigned short id, std::function<void (Peer*)> cb);
        
        template <typename T0>
        void on_message(unsigned short id, std::function<void (Peer*, T0)> cb);
        
        template <typename T0, typename T1>
        void on_message(unsigned short id, std::function<void (Peer*, T0, T1)> cb);
        
        void send(Peer* peer, unsigned char message);
        
        template <typename T0>
        void send(Peer* peer, unsigned char message, T0 v0);
        
        template <typename T0, typename T1>
        void send(Peer* peer, unsigned char message, T0 v0, T1 v1);
        
        void generic_send(Peer* peer, unsigned char message, std::function<void (std::ostream&)> pack_data);
        
        void broadcast(unsigned char message);
        
        template <typename T0>
        void broadcast(unsigned char message, T0 v0);
        
        template <typename T0, typename T1>
        void broadcast(unsigned char message, T0 v0, T1 v1);
        
        void generic_broadcast(unsigned char message, std::function<void (std::ostream&)> pack_data);
        
        void run();
        
        void stop();
    
    private:
        unsigned char id;
        unsigned char version;
        c9y::UdpSocket* socket;
        c9y::Timer*     timer;
        std::function<void (Peer*)> connect_cb;
        std::function<void (Peer*)> disconnect_cb;        
        
        std::vector<Peer*> peers;
        std::map<unsigned int, std::function<void (Peer*, std::istream&)>> messages;
        
        void handle_message(const c9y::IpAddress& address, const char* data, size_t len);
        std::string build_message(unsigned char message, std::function<void (std::ostream&)> pack_data);
        
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
    void Node::send(Peer* peer, unsigned char message, T0 v0)
    {
        generic_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }
    
    template <typename T0, typename T1>
    void Node::send(Peer* peer, unsigned char message, T0 v0, T1 v1)
    {
        generic_send(peer, message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
        });
    }
    
    template <typename T0>
    void Node::broadcast(unsigned char message, T0 v0)
    {
        generic_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
        });
    }

    template <typename T0, typename T1>
    void Node::broadcast(unsigned char message, T0 v0, T1 v1)
    {
        generic_broadcast(message, [&] (std::ostream& os) {
            pack(os, v0);
            pack(os, v1);
        });
    }
}

#endif

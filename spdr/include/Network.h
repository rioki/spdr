// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NETWORK_H_
#define _SPDR_NETWORK_H_

#include <queue>
#include <sigc++/signal.h>
#include <c9y/Thread.h>
#include <c9y/Mutex.h>
#include <c9y/Condition.h>

#include "Node.h"
#include "Address.h"
#include "UdpSocket.h"
#include "Message.h"

#include "MessageQueue.h"
#include "NodeList.h"

namespace spdr
{
    class Network
    {
    public:
    
        Network();
        
        Network(unsigned short connect_port);
        
        ~Network();
        
        unsigned short get_connect_port() const;
        
        bool accepts_connections() const;
        
        NodePtr get_this_node() const;
        
        sigc::signal<void, NodePtr> node_connected;
        
        NodePtr connect(const Address& address);
        
        sigc::signal<void, MessagePtr> message_recived;
        
        void send(MessagePtr message);
    
    private:
        unsigned short connect_port;
        NodePtr this_node;
        
        NodeList nodes;
                
        UdpSocket socket;
        
        bool running;
        c9y::Thread worker_thread; 
        
        MessageQueue send_queue;
        
        sigc::signal<void, MessagePtr> internal_message_recived;
        
        void init(unsigned short connect_port);
        
        NodePtr create_node(const Address& address);
        void remove_node(NodePtr node);
        NodePtr get_node_from_address(const Address& address);
                
        void main();
        
        void send_message(MessagePtr msg);
        
        MessagePtr recive_message();              
        
        MessagePtr create_message(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload);
        void handle_internal_message(MessagePtr msg);
        
        void handle_connect(MessagePtr msg);
    
        Network(const Network&);
        const Network& operator = (const Network&);
    
    friend class Connector;
    };
}

#endif

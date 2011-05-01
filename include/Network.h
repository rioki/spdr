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
        std::vector<NodePtr> nodes;
                
        UdpSocket socket;
        
        bool running;
        c9y::Thread worker_thread; 
        
        c9y::Mutex send_queue_mutex;
        std::queue<MessagePtr> send_queue;
        
        c9y::Condition connect_condition;
        
        void init(unsigned short connect_port);
        void main();
        
        MessagePtr get_next_message();
        void send_message(MessagePtr msg);
        MessagePtr recive_message();      
        NodePtr get_node_from_address(const Address& address);
        void handle_internal_message(MessagePtr msg);
        
        void handle_connect(MessagePtr msg);
        void handle_ack_connect(MessagePtr msg);
    
        Network(const Network&);
        const Network& operator = (const Network&);
    };
}

#endif

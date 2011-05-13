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
    
        Network(unsigned int protocol_id);
        
        Network(unsigned int protocol_id, unsigned short connect_port);
        
        ~Network();
        
        unsigned int get_protocol_id() const;
        
        unsigned short get_connect_port() const;
        
        bool accepts_connections() const;
        
        NodePtr get_this_node() const;
        
        /**
         * Signal that is emitted when a node is connected.
         **/
        sigc::signal<void, NodePtr> node_connected;
        
        /**
         * Signal that is emitted when a node timesout.
         **/
        sigc::signal<void, NodePtr> node_timeout;
        
        /**
         * Signal that is emitted when a node was disconnected.
         **/
        sigc::signal<void, NodePtr> node_disconnected;
        
        /**
         * Initiate connection to an other node. 
         **/
        NodePtr connect(const Address& address);
        
        /**
         * Signal that is emitted when a message is recived. 
         **/
        sigc::signal<void, MessagePtr> message_recived;
        
        /**
         * Send a message. 
         **/
        void send(MessagePtr message);
    
    private:
        unsigned int protocol_id;
        unsigned short connect_port;
        NodePtr this_node;
        
        NodeList nodes;
        
        UdpSocket socket;
        
        bool running;
        c9y::Thread worker_thread; 
        
        MessageQueue send_queue;
        
        sigc::signal<void, MessagePtr> internal_message_recived;        
        
        void init(unsigned short connect_port);
        
        // node handling
        NodePtr create_node(const Address& address);
        void remove_node(NodePtr node);
        NodePtr get_node_from_address(const Address& address);
        
        // worker thread
        void main();        
        void send_message(MessagePtr msg);        
        MessagePtr recive_message();
        MessagePtr create_message(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload);
        void handle_internal_message(MessagePtr msg);        
        void handle_connect(MessagePtr msg);
        void handle_connection_accepted(MessagePtr msg);
        void handle_connection_rejected(MessagePtr msg);
        
        void check_node_timeout();
    
        Network(const Network&);
        const Network& operator = (const Network&);
    
    friend class Connector;
    };
}

#endif

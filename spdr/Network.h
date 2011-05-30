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
#include "MessageFactory.h"

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
        
        std::tr1::shared_ptr<Node> get_this_node() const;
        
        /**
         * Signal that is emitted when a node is connected.
         **/
        sigc::signal<void, std::tr1::shared_ptr<Node> > node_connected;
        
        /**
         * Signal that is emitted when a node timesout.
         **/
        sigc::signal<void, std::tr1::shared_ptr<Node> > node_timeout;
        
        /**
         * Signal that is emitted when a node was disconnected.
         **/
        sigc::signal<void, std::tr1::shared_ptr<Node> > node_disconnected;
        
        /**
         * Initiate connection to an other node. 
         **/
        std::tr1::shared_ptr<Node> connect(const Address& address);
        
        /**
         * Signal that is emitted when a message is recived. 
         **/
        sigc::signal<void, std::tr1::shared_ptr<Message> > message_recived;
        
        template <typename Type>
        void register_message();
        
        /**
         * Send a message. 
         **/
        void send(std::tr1::shared_ptr<Node> node, std::tr1::shared_ptr<Message> message);
    
    private:
        unsigned int protocol_id;
        unsigned short connect_port;
        NodePtr this_node;
        
        NodeList nodes;
        
        UdpSocket socket;
        
        bool running;
        c9y::Thread worker_thread; 
        
        MessageQueue send_queue;
        
        MessageFactory message_factory;       
        
        void init(unsigned short connect_port);
        
        // node handling
        NodePtr create_node(const Address& address);
        void remove_node(NodePtr node);
        NodePtr get_node_from_address(const Address& address);
        
        // worker thread
        void main();        
        void send_message(std::tr1::shared_ptr<Message> msg);        
        std::tr1::shared_ptr<Message> recive_message();
        std::tr1::shared_ptr<Message> create_message(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload);
        void handle_internal_message(std::tr1::shared_ptr<Message> msg);        
        void handle_connect(std::tr1::shared_ptr<Message> msg);
        void handle_connection_accepted(std::tr1::shared_ptr<Message> msg);
        void handle_connection_rejected(std::tr1::shared_ptr<Message> msg);
        
        void check_node_timeout();
        void handle_keep_alive();
    
        Network(const Network&);
        const Network& operator = (const Network&);
    };
    
//------------------------------------------------------------------------------
    template <typename Type>
    void Network::register_message()
    {
        std::tr1::shared_ptr<Message> message(new Type);
        message_factory.add(message);
    }
}

#endif

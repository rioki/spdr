// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <sigc++/signal.h>
#include <queue>
#include <list>
#include <map>

#include <c9y/Thread.h>
#include <c9y/Mutex.h>
#include <c9y/Lock.h>

#include "Address.h"
#include "UdpSocket.h"

#include "PeerInfo.h"
#include "Message.h"

namespace spdr
{
    typedef Message* (*MessageCreator)();
    
    template <typename Type>
    struct MessageFactory
    {
        static Message* create()
        {
            return new Type;
        }
    };
    
    /**
     * Network Network
     **/
    class Network
    {
    public:
    
        /**
         * Create a client network node.
         **/
        Network(unsigned int protocol_id);
        
        /**
         * Create a server network node.
         **/
        Network(unsigned int protocol_id, unsigned int port);
        
        /**
         * Destroy 
         **/
        ~Network();
        
        /**
         * Get the protocoll id.
         **/
        unsigned int get_protocol_id() const;
        
        /**
         * Connect to a peer.
         **/
        PeerInfo connect(Address address);

        /**
         * Get the signal that is emitted when a connection is established.
         **/
        sigc::signal<void, PeerInfo>& get_connect_signal();
        
        /**
         * Get the signal that is emitted when a connection is terminated.
         **/
        sigc::signal<void, PeerInfo>& get_disconnect_signal();
        
        template <typename Type>
        void register_message(unsigned int id)
        {
            c9y::Lock<c9y::Mutex> lock(message_map_mutex);
            message_map[id] = &MessageFactory<Type>::create;
        }
        
        /**
         * Send a message.
         **/
        void send(const PeerInfo& info, Message* message);
        
        sigc::signal<void, PeerInfo, Message&>& get_message_signal();
    
    private:
        unsigned int protocol_id;
        UdpSocket socket;
        bool running;
        c9y::Thread worker;

        std::list<PeerInfo*> connected_nodes;
        c9y::Mutex connected_nodes_mutex;
        
        std::queue<std::tr1::tuple<PeerInfo, Message*> > send_queue;
        c9y::Mutex send_queue_mutex;
        
        std::map<unsigned int, MessageCreator> message_map;
        c9y::Mutex message_map_mutex;
        
        sigc::signal<void, PeerInfo> connect_signal;
        sigc::signal<void, PeerInfo> disconnect_signal;
        sigc::signal<void, PeerInfo, Message&> message_signal;
        
        void register_system_messages();
        
        void worker_main();
        
        void do_send();
        void do_recive();
        void do_keep_alive();
        void do_timeout();
        
        PeerInfo* get_info(const Address& address);
        Message* create_message(unsigned int id);
        void handle_system_message(PeerInfo info, Message& message);
    
        Network(const Network&);
        const Network& operator = (const Network&);
    };
}

#endif

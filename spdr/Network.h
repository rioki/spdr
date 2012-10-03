// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <sigc++/signal.h>
#include <queue>
#include <list>
#include <map>
#include <tuple>

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
        PeerInfoPtr connect(Address address);
        
        /**
         * Disconnect from a peer.
         **/
        void disconnect(Address address);

        /**
         * Get the signal that is emitted when a connection is established.
         **/
        sigc::signal<void, PeerInfoPtr>& get_connect_signal();
        
        /**
         * Get the signal that is emitted when a connection is terminated.
         **/
        sigc::signal<void, PeerInfoPtr>& get_disconnect_signal();
        
        template <typename Type>
        void register_message(unsigned int id)
        {
            c9y::Lock<c9y::Mutex> lock(message_map_mutex);
            message_map[id] = &MessageFactory<Type>::create;
        }
        
        /**
         * Send a message.
         **/
        template <typename Type>
        void send(PeerInfoPtr peer, Type message)
        {
            c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
            send_queue.push(std::make_tuple(peer, new Type(message)));
        }
        
        sigc::signal<void, PeerInfoPtr, Message&>& get_message_signal();
    
    private:
        unsigned int protocol_id;
        UdpSocket socket;
        bool running;
        c9y::Thread worker;

        std::list<PeerInfoPtr> peers;
        c9y::Mutex peers_mutex;
        
        std::queue<std::tuple<PeerInfoPtr, Message*> > send_queue;
        c9y::Mutex send_queue_mutex;
        
        std::vector<std::tuple<PeerInfoPtr, Message*> > sent_messages;
        c9y::Mutex sent_messages_mutex;
        
        std::map<unsigned int, MessageCreator> message_map;
        c9y::Mutex message_map_mutex;
        
        sigc::signal<void, PeerInfoPtr> connect_signal;
        sigc::signal<void, PeerInfoPtr> disconnect_signal;
        sigc::signal<void, PeerInfoPtr, Message&> message_signal;
        
        void register_system_messages();
        
        void worker_main();
        
        void do_send();
        void do_recive();
        void handle_incomming_acks(PeerInfoPtr peer, unsigned int sequence_number, unsigned int last_ack, unsigned int ack_field);
        void do_keep_alive();
        void do_timeout(); 
        void do_reliable_messages();
        
        PeerInfoPtr get_info(const Address& address, bool create = true);
        Message* create_message(unsigned int id);
    
        Network(const Network&);
        const Network& operator = (const Network&);
    };
}

#endif

// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <iosfwd>
#include <tr1/memory>
#include <string>

#include "Address.h"

namespace spdr
{
    /**
     * Network node 
     **/
    class Node
    {
    public:
    
        enum State
        {
            CONNECTING,
            CONNECTED,
            TIMEOUT,
            DISCONNECTED
        };
        
        /**
         * Get the state of the node. 
         **/
        State get_state() const;
        
        /**
         * Get the address of the node. 
         **/
        const Address& get_address() const;
        
        const std::string& get_user_name() const;
        
        /**
         * Get the time of the last message. 
         **/
        unsigned int get_last_message_recived() const;
    
        unsigned int get_last_message_sent() const;
    
    private:
        State state;
        Address address;
        std::string user_name;
        unsigned int last_message_recived;
        unsigned int last_message_sent;
        
        Node();
        
        Node(const Address& address);    

        void set_state(State value);
        
    friend class Network;
    };
    
    /**
     * Stream operator for a Node's state. 
     **/
    std::ostream& operator << (std::ostream& os, Node::State state);
    
    typedef std::tr1::shared_ptr<Node> NodePtr;
}

#endif

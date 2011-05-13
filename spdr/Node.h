// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <iosfwd>
#include <tr1/memory>

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
        
        /**
         * Get the time of the last message. 
         **/
        unsigned int get_last_message_recived() const;
    
    private:
        State state;
        Address address;
        unsigned int last_message_recived;
        
        Node();
        
        Node(const Address& address);    

        void set_state(State value);
        
    friend class Network;
    };
    
    std::ostream& operator << (std::ostream& os, Node::State state);
    
    typedef std::tr1::shared_ptr<Node> NodePtr;
}

#endif

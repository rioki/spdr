// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "Message.h"

namespace spdr
{
    /**
     * Message used to establish connections.
     **/
    class Connect : public Message
    {
    public:
        /**
         * Send Constructor
         **/
        Connect(NodePtr to, unsigned int protocol_id);
        
        /**
         * Recive Constructor
         **/
        Connect(NodePtr to, NodePtr from, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> get_payload() const; 

        unsigned int get_protocol_id() const;
    
    private:
        unsigned int protocol_id;
    };    
}

#endif

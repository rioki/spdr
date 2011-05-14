// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _CONNECTION_REJECTED_H_
#define _CONNECTION_REJECTED_H_

#include "Message.h"

namespace spdr
{
    /**
     * Message used when a connection is accepted.
     **/
    class ConnectionRejected : public Message
    {
    public:
        /**
         * Send Constructor
         **/
        ConnectionRejected(NodePtr to);
        
        /**
         * Recive Constructor
         **/
        ConnectionRejected(NodePtr to, NodePtr from, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> get_payload() const; 
    
    private:
        
    };    
}

#endif

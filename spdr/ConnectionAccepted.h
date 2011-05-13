// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _CONNECTION_ACCEPTED_H_
#define _CONNECTION_ACCEPTED_H_

#include "Message.h"

namespace spdr
{
    /**
     * Message used when a connection is accepted.
     **/
    class ConnectionAccepted : public Message
    {
    public:
        /**
         * Send Constructor
         **/
        ConnectionAccepted(NodePtr to);
        
        /**
         * Recive Constructor
         **/
        ConnectionAccepted(NodePtr to, NodePtr from, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> get_payload() const; 
    
    private:
        
    };    
}

#endif

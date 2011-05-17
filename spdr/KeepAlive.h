// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _KEEP_ALIVE_H_
#define _KEEP_ALIVE_H_

#include "Message.h"

namespace spdr
{
    /**
     * Message used when a connection is accepted.
     **/
    class KeepAlive : public Message
    {
    public:
        /**
         * Send Constructor
         **/
        KeepAlive(NodePtr to);
        
        /**
         * Recive Constructor
         **/
        KeepAlive(NodePtr to, NodePtr from, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> get_payload() const; 
    
    private:
        
    };    
}

#endif

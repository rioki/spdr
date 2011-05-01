// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _CONNECT_MESSAGE_H_
#define _CONNECT_MESSAGE_H_

#include "Message.h"

namespace spdr
{
    /**
     * Message used to establish connections.
     **/
    class ConnectMessage : public Message
    {
    public:
        /**
         * Send Constructor
         **/
        ConnectMessage(NodePtr to, unsigned int version);
        
        /**
         * Recive Constructor
         **/
        ConnectMessage(NodePtr to, NodePtr from, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> get_payload() const; 

        unsigned int get_version() const;
    
    private:
        unsigned int version;
    };    
}

#endif

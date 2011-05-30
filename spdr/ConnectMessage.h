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
    class ConnectMessage : public Message
    {
    public:
        /**
         * Deault Constructor
         **/
        ConnectMessage();
        
        /**
         * Initialize ConnectionMessage
         **/
        ConnectMessage(unsigned int protocol_id);
        
        virtual ConnectMessage* clone() const;
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> encode() const;
    
        virtual void decode(const std::vector<char>& paylod);

        unsigned int get_protocol_id() const;
        
    private:
        unsigned int protocol_id;
    };    
}

#endif

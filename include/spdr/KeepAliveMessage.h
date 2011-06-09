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
    class KeepAliveMessage : public Message
    {
    public:
        /**
         * Deault Constructor
         **/
        KeepAliveMessage();
        
        /**
         * Initialize ConnectionMessage
         **/
        KeepAliveMessage(unsigned int protocol_id);
        
        virtual KeepAliveMessage* clone() const;
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> encode() const;
    
        virtual void decode(const std::vector<char>& paylod);
    
    private:
        
    };    
}

#endif

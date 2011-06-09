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
    class AcceptMessage : public Message
    {
    public:
        /**
         * Deault Constructor
         **/
        AcceptMessage();
        
        /**
         * Initialize ConnectionMessage
         **/
        AcceptMessage(unsigned int protocol_id);
        
        virtual AcceptMessage* clone() const;
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> encode() const;
    
        virtual void decode(const std::vector<char>& paylod);
    
    private:
        
    };    
}

#endif

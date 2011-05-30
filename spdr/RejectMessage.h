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
    class RejectMessage : public Message
    {
    public:
        /**
         * Deault Constructor
         **/
        RejectMessage();
        
        virtual RejectMessage* clone() const;
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> encode() const;
    
        virtual void decode(const std::vector<char>& paylod);
    
    private:
        
    };    
}

#endif

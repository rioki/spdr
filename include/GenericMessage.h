// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_GENERIC_MESSAGE_H_
#define _SPDR_GENERIC_MESSAGE_H_

#include "Message.h"

namespace spdr
{
    /**
     * Generic Message
     **/
    class GenericMessage : public Message
    {
    public:
    
        GenericMessage(NodePtr to, unsigned int type, const std::vector<char>& payload);
        
        GenericMessage(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload);
        
        virtual unsigned int get_type() const;
        
        virtual std::vector<char> get_payload() const;        
    
    private:
        unsigned int type;
        std::vector<char> payload;
    };    
}

#endif

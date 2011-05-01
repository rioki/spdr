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
    
        GenericMessage(NodePtr to, unsigned int type, const std::string& payload);
        
        GenericMessage(NodePtr to, NodePtr from, unsigned int type, const std::string& payload);
        
        virtual unsigned int get_type() const;
        
        virtual unsigned int get_payload_size() const;
        
        virtual std::string get_payload() const;        
    
    private:
        unsigned int type;
        std::string payload;
    };    
}

#endif

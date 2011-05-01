// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "GenericMessage.h"

namespace spdr
{
//------------------------------------------------------------------------------
    GenericMessage::GenericMessage(NodePtr to, unsigned int t, const std::string& p)
    : Message(to), type(t), payload(p) {}
    
    
//------------------------------------------------------------------------------
    GenericMessage::GenericMessage(NodePtr to, NodePtr from, unsigned int t, const std::string& p)
    : Message(to, from), type(t), payload(p) {}
    
//------------------------------------------------------------------------------
    unsigned int GenericMessage::get_type() const
    {
        return type;
    }
    
//------------------------------------------------------------------------------
    unsigned int GenericMessage::get_payload_size() const
    {
        return payload.size();
    }
    
//------------------------------------------------------------------------------
    std::string GenericMessage::get_payload() const
    {
        return payload;
    }
}

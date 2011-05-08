// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "GenericMessage.h"

namespace spdr
{
//------------------------------------------------------------------------------
    GenericMessage::GenericMessage(NodePtr to, unsigned int t, const std::vector<char>& p)
    : Message(to), type(t), payload(p) {}
    
    
//------------------------------------------------------------------------------
    GenericMessage::GenericMessage(NodePtr to, NodePtr from, unsigned int t, const std::vector<char>& p)
    : Message(to, from), type(t), payload(p) {}
    
//------------------------------------------------------------------------------
    unsigned int GenericMessage::get_type() const
    {
        return type;
    }
    
//------------------------------------------------------------------------------
    std::vector<char> GenericMessage::get_payload() const
    {
        return payload;
    }
}

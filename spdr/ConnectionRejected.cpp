// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "ConnectionRejected.h"

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    ConnectionRejected::ConnectionRejected(NodePtr to)
    : Message(to) {}
    
    
//------------------------------------------------------------------------------
    ConnectionRejected::ConnectionRejected(NodePtr to, NodePtr from, const std::vector<char>& payload)
    : Message(to, from) {}
    
//------------------------------------------------------------------------------
    unsigned int ConnectionRejected::get_type() const
    {
        return CONNECTION_REJECTED;
    }

//------------------------------------------------------------------------------
    std::vector<char> ConnectionRejected::get_payload() const
    {
        return std::vector<char>();
    }
}

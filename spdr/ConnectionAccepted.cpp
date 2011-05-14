// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "ConnectionAccepted.h"

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    ConnectionAccepted::ConnectionAccepted(NodePtr to)
    : Message(to) {}
    
    
//------------------------------------------------------------------------------
    ConnectionAccepted::ConnectionAccepted(NodePtr to, NodePtr from, const std::vector<char>& payload)
    : Message(to, from) {}
    
//------------------------------------------------------------------------------
    unsigned int ConnectionAccepted::get_type() const
    {
        return CONNECTION_ACCEPTED;
    }

//------------------------------------------------------------------------------
    std::vector<char> ConnectionAccepted::get_payload() const
    {
        return std::vector<char>();
    }
}

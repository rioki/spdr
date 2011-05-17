// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "KeepAlive.h"

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    KeepAlive::KeepAlive(NodePtr to)
    : Message(to) {}    
    
//------------------------------------------------------------------------------
    KeepAlive::KeepAlive(NodePtr to, NodePtr from, const std::vector<char>& payload)
    : Message(to, from) {}
    
//------------------------------------------------------------------------------
    unsigned int KeepAlive::get_type() const
    {
        return KEEP_ALIVE;
    }

//------------------------------------------------------------------------------
    std::vector<char> KeepAlive::get_payload() const
    {
        return std::vector<char>();
    }
}

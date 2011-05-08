// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Node.h"

namespace spdr
{
//------------------------------------------------------------------------------    
    const Address& Node::get_address() const
    {
        return address;
    }
    
//------------------------------------------------------------------------------    
    Node::Node() {}

//------------------------------------------------------------------------------    
    Node::Node(const Address& a)
    : address(a) {}
}

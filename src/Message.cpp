// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Message.h"

namespace spdr
{
//------------------------------------------------------------------------------    
    Message::Message(NodePtr t)
    : to(t) {}
       
//------------------------------------------------------------------------------
    Message::Message(NodePtr t, NodePtr f)
    : to(t), from(f) {}

//------------------------------------------------------------------------------    
    Message::~Message() {}

//------------------------------------------------------------------------------    
    NodePtr Message::get_from() const
    {
        return from;
    }
    
//------------------------------------------------------------------------------    
    NodePtr Message::get_to() const
    {
        return to;
    }
}

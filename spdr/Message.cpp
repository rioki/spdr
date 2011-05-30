// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Message.h"

namespace spdr
{
//------------------------------------------------------------------------------    
    Message::Message() {}
    
//------------------------------------------------------------------------------    
    Message::~Message() {}
    
//------------------------------------------------------------------------------
    void Message::set_to(std::tr1::shared_ptr<Node> value)
    {
        to = value;
    }
    
//------------------------------------------------------------------------------
    std::tr1::shared_ptr<Node> Message::get_to() const
    {
        return to;
    }
    
//------------------------------------------------------------------------------
    void Message::set_from(std::tr1::shared_ptr<Node> value)
    {
        from = value;
    }
    
//------------------------------------------------------------------------------
    std::tr1::shared_ptr<Node> Message::get_from() const
    {
        return from;
    }
}

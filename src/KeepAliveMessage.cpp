// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "KeepAliveMessage.h"

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    KeepAliveMessage::KeepAliveMessage() {}    
    
//------------------------------------------------------------------------------
    KeepAliveMessage::KeepAliveMessage(unsigned int pi) {}

//------------------------------------------------------------------------------    
    KeepAliveMessage* KeepAliveMessage::clone() const
    {
        return new KeepAliveMessage(*this);
    }

//------------------------------------------------------------------------------
    unsigned int KeepAliveMessage::get_type() const
    {
        return KEEP_ALIVE_MESSAGE;
    }

//------------------------------------------------------------------------------
    std::vector<char> KeepAliveMessage::encode() const
    {
        return std::vector<char>();
    }

//------------------------------------------------------------------------------
    void KeepAliveMessage::decode(const std::vector<char>& paylod)
    {
    }
}

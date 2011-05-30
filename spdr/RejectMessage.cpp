// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "RejectMessage.h"

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    RejectMessage::RejectMessage() {}    

//------------------------------------------------------------------------------    
    RejectMessage* RejectMessage::clone() const
    {
        return new RejectMessage(*this);
    }

//------------------------------------------------------------------------------
    unsigned int RejectMessage::get_type() const
    {
        return REJECT_MESSAGE;
    }

//------------------------------------------------------------------------------
    std::vector<char> RejectMessage::encode() const
    {
        return std::vector<char>();
    }

//------------------------------------------------------------------------------
    void RejectMessage::decode(const std::vector<char>& payload)
    {
    }
}

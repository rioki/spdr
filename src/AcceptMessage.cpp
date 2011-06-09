// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "AcceptMessage.h"

#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    AcceptMessage::AcceptMessage() {}    
    
//------------------------------------------------------------------------------
    AcceptMessage::AcceptMessage(unsigned int pi) {}

//------------------------------------------------------------------------------    
    AcceptMessage* AcceptMessage::clone() const
    {
        return new AcceptMessage(*this);
    }

//------------------------------------------------------------------------------
    unsigned int AcceptMessage::get_type() const
    {
        return ACCEPT_MESSAGE;
    }

//------------------------------------------------------------------------------
    std::vector<char> AcceptMessage::encode() const
    {
        return std::vector<char>();
    }

//------------------------------------------------------------------------------
    void AcceptMessage::decode(const std::vector<char>& payload)
    {
    }
}

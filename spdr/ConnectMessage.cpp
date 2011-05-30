// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "ConnectMessage.h"

#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage()
    : protocol_id(0) {}
    
    
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage(unsigned int pi)
    : protocol_id(pi) {}

//------------------------------------------------------------------------------    
    ConnectMessage* ConnectMessage::clone() const
    {
        return new ConnectMessage(*this);
    }

//------------------------------------------------------------------------------
    unsigned int ConnectMessage::get_type() const
    {
        return CONNECT_MESSAGE;
    }

//------------------------------------------------------------------------------
    std::vector<char> ConnectMessage::encode() const
    {
        std::vector<char> paylod;
        
        musli::MemoryPacker packer(paylod);
        packer << protocol_id;
        
        return paylod;
    }

//------------------------------------------------------------------------------
    void ConnectMessage::decode(const std::vector<char>& paylod)
    {
        musli::MemoryUnpacker unpacker(paylod);
        unpacker >> protocol_id;
    }

//------------------------------------------------------------------------------    
    unsigned int ConnectMessage::get_protocol_id() const
    {
        return protocol_id;
    }
}
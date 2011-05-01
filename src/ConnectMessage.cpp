// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "ConnectMessage.h"

#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage(NodePtr to, unsigned int v)
    : Message(to), version(v) {}
    
    
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage(NodePtr to, NodePtr from, const std::vector<char>& payload)
    : Message(to, from), version(0) 
    {
        // FIXME: remove const_cast once fixed in musli
        musli::MemoryUnpacker unpacker(const_cast<std::vector<char>&>(payload));
        unpacker >> version;
    }
    
//------------------------------------------------------------------------------
    unsigned int ConnectMessage::get_type() const
    {
        return CONNECT;
    }
    
//------------------------------------------------------------------------------
    std::vector<char> ConnectMessage::get_payload() const
    {
        std::vector<char> payload;
        musli::MemoryPacker unpacker(payload);
        unpacker << version;
        return payload;
    }

//------------------------------------------------------------------------------    
    unsigned int ConnectMessage::get_version() const
    {
        return version;
    }
}

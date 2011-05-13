// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Connect.h"

#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    Connect::Connect(NodePtr to, unsigned int p)
    : Message(to), protocol_id(p) {}
    
    
//------------------------------------------------------------------------------
    Connect::Connect(NodePtr to, NodePtr from, const std::vector<char>& payload)
    : Message(to, from), protocol_id(0) 
    {
        // FIXME: remove const_cast once fixed in musli
        musli::MemoryUnpacker unpacker(const_cast<std::vector<char>&>(payload));
        unpacker >> protocol_id;
    }
    
//------------------------------------------------------------------------------
    unsigned int Connect::get_type() const
    {
        return CONNECT;
    }

//------------------------------------------------------------------------------
    std::vector<char> Connect::get_payload() const
    {
        std::vector<char> payload;
        musli::MemoryPacker unpacker(payload);
        unpacker << protocol_id;
        return payload;
    }

//------------------------------------------------------------------------------    
    unsigned int Connect::get_protocol_id() const
    {
        return protocol_id;
    }
}

// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "ConnectMessage.h"

#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"

namespace spdr
{
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage() {}
    
    
//------------------------------------------------------------------------------
    ConnectMessage::ConnectMessage(const std::string& u, const std::string& p)
    : user(u), pass(p) {}

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
        packer << user << pass;
        
        return paylod;
    }

//------------------------------------------------------------------------------
    void ConnectMessage::decode(const std::vector<char>& paylod)
    {
        musli::MemoryUnpacker unpacker(paylod);
        unpacker >> user >> pass;
    }

//------------------------------------------------------------------------------    
    const std::string& ConnectMessage::get_user() const
    {
        return user;
    }

//------------------------------------------------------------------------------        
    const std::string& ConnectMessage::get_pass() const
    {
        return pass;
    }
}
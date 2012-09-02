// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "KeepAliveMessage.h"

namespace spdr
{
    KeepAliveMessage::KeepAliveMessage() {}
    
    KeepAliveMessage::~KeepAliveMessage() {}
    
    unsigned int KeepAliveMessage::get_id() const
    {
        return KEEP_ALIVE_MESSAGE;
    }
    
    bool KeepAliveMessage::is_reliable() const
    {
        return false;
    }
    
    void KeepAliveMessage::pack(std::ostream& os) const {}
    
    void KeepAliveMessage::unpack(std::istream& is) {}
}

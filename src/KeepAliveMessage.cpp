// Iced Blue
// Copyright 2011 Sean Farrell

#include "KeepAliveMessage.h"

#include "SystemMessageId.h"

namespace spdr
{
    KeepAliveMessage::KeepAliveMessage() {}
    
    KeepAliveMessage::~KeepAliveMessage() {}
    
    unsigned int KeepAliveMessage::get_id() const
    {
        return 0;
    }
    
    void KeepAliveMessage::pack(std::ostream& os) const {}
    
    void KeepAliveMessage::unpack(std::istream& is) {}
}

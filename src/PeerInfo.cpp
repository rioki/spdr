// Iced Blue
// Copyright 2011 Sean Farrell

#include "PeerInfo.h"

namespace spdr
{
    PeerInfo::PeerInfo() {}

    PeerInfo::PeerInfo(const Address& a, unsigned int now)
    : address(a), last_message_recived(now), last_message_sent(0) {}
    
    const Address& PeerInfo::get_address() const
    {
        return address;
    }
}

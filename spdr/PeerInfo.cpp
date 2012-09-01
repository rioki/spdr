// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "PeerInfo.h"

namespace spdr
{
    PeerInfo::PeerInfo()
    : connected(false), disconnecting(false), last_message_recived(0), last_message_sent(0) {}

    PeerInfo::PeerInfo(const Address& a, unsigned int now)
    : address(a), last_message_recived(now), last_message_sent(0) {}
    
    const Address& PeerInfo::get_address() const
    {
        return address;
    }
    
    bool PeerInfo::is_connected() const
    {
        return connected;
    }
}

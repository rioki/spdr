// Iced Blue
// Copyright 2011 Sean Farrell

#ifndef _IBNET_NODE_INFO_H_
#define _IBNET_NODE_INFO_H_

#include "Address.h"

namespace spdr
{
    class PeerInfo
    {
    public:
        PeerInfo();
    
        PeerInfo(const Address& address, unsigned int now);
        
        const Address& get_address() const;

    private:
        Address address;
        unsigned int last_message_recived;
        unsigned int last_message_sent;
    
    friend class Network;
    };
}

#endif

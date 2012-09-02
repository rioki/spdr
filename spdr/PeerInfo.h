// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifndef _SPDR_NODE_INFO_H_
#define _SPDR_NODE_INFO_H_

#include "Address.h"

namespace spdr
{
    class PeerInfo
    {
    public:
        PeerInfo();
    
        PeerInfo(const Address& address, unsigned int now);
        
        const Address& get_address() const;
        
        bool is_connected() const;

    private:
        bool connected;
        bool disconnecting;
        Address address;
        unsigned int last_message_recived;
        unsigned int last_message_sent;
        unsigned int last_sequence_number;
        unsigned int remote_sequence_number;
        unsigned int ack_field;
    
    friend class Network;
    };
}

#endif


#include "Peer.h"

namespace spdr
{
    const std::string& Peer::get_address() const
    {
        return address;
    }
    
    unsigned short Peer::get_port() const
    {
        return port;
    }
    
    Peer::Peer(const std::string& a, unsigned int p, clock_t now)
    : address(a), port(p), 
      last_message_recived(now), last_message_sent(0),
      last_sequence_number(0), remote_sequence_number(0), 
      ack_field(0) {}
}

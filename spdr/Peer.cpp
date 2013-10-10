
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
    
    Peer::Peer(const std::string& a, unsigned int p)
    : address(a), port(p) {}    
}

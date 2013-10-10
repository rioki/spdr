
#ifndef _PEER_H_
#define _PEER_H_

#include <string>

namespace spdr
{
    class Peer
    {
    public:
        
        const std::string& get_address() const;
        
        unsigned short get_port() const;
        
    private:
        Peer(const std::string& address, unsigned int port);
    
        std::string address;
        unsigned short port;
        
    friend class Node;
    };
}

#endif

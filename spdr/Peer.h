
#ifndef _PEER_H_
#define _PEER_H_

#include <string>
#include <ctime>

namespace spdr
{
    class Peer
    {
    public:
        
        const std::string& get_address() const;
        
        unsigned short get_port() const;
        
    private:
        Peer(const std::string& address, unsigned int port, clock_t now);
    
        std::string address;
        unsigned short port;
        
        clock_t  last_message_recived;
        clock_t  last_message_sent;
        unsigned int  last_sequence_number;
        unsigned int  remote_sequence_number;
        unsigned char ack_field;
        
    friend class Node;
    };
}

#endif

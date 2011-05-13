// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_UDP_SOCKET_H_
#define _SPDR_UDP_SOCKET_H_

#include "Address.h"

#include <vector>
#include <tr1/tuple>

namespace spdr
{
    /**
     * UDP Socket
     **/
    class UdpSocket
    {
    public:
        /**
         * Create a socket bound to a random port.
         **/
        UdpSocket();
        
        /**
         * Create a socket bound to a specific port.
         **/
        UdpSocket(unsigned short port);
        
        /**
         * Destructor
         **/
        ~UdpSocket();
        
        /**
         * Get the maximum packet size this UDP socket will accept. 
         **/
        unsigned int get_max_packet_size() const;
        
        /**
         * Send a packet.
         **/
        void send(const Address& address, const std::vector<char>& data);
        
        /**
         * Recive a packet.
         **/
        std::tr1::tuple<Address, std::vector<char> > recive();
        
    private:
        int handle;
        
        void init(unsigned short port);
    
        UdpSocket(const UdpSocket&);
        const UdpSocket& operator = (const UdpSocket&);
    };
}

#endif

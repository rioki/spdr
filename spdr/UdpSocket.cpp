// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "UdpSocket.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <iostream>
#include <stdexcept>
#include <vector>

#include "debug.h"

namespace spdr
{
//------------------------------------------------------------------------------
    UdpSocket::UdpSocket() 
    {
        init(0);
    }

//------------------------------------------------------------------------------    
    UdpSocket::UdpSocket(unsigned short port)
    {
        init(port);
    }
        
//------------------------------------------------------------------------------
    UdpSocket::~UdpSocket() 
    {
        TRACE("Closed handle 0x%04X.", handle);
#if _WIN32
        closesocket(handle);
#else
        close(handle);
#endif    
    }

//------------------------------------------------------------------------------    
    unsigned int UdpSocket::get_max_packet_size() const
    {
        return 512;
    }
    
//------------------------------------------------------------------------------
    void UdpSocket::send(const Address& address, const std::string& data)
    {
        if (data.size() > get_max_packet_size())
        {
            throw std::invalid_argument("the data execes the maximum packet size");
        }
        if (data.empty())
        {
            throw std::invalid_argument("the data is empty");
        }              
        
        sockaddr_in c_adr = address.get_c_obj();
        int sent_bytes = sendto(handle, &data[0], data.size(), 0, (sockaddr*)&c_adr, sizeof(sockaddr_in));
        if (sent_bytes != data.size())
        {
            throw std::runtime_error("failed to send packet");
        }
        
        TRACE("Sent %d bytes to %d.%d.%d.%d:%d. (0x%04X)", data.size(), address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port(), handle);
    }
    
//------------------------------------------------------------------------------
    std::tuple<Address, std::string> UdpSocket::recive()
    {
        std::string buff(get_max_packet_size(), 0);    
        sockaddr_in c_adr;
        int c_adr_len = sizeof(sockaddr_in);
        
        int received_bytes = recvfrom(handle, &buff[0], buff.size(), 0, (sockaddr*)&c_adr, &c_adr_len);
        
        if (received_bytes <= 0)
        {
            return std::make_tuple(Address(), std::string());
        }
        else
        {
            Address address(c_adr);
            buff.resize(received_bytes);
            
            TRACE("Recived %d bytes to %d.%d.%d.%d:%d. (0x%04X)", buff.size(), address.get_a(), address.get_b(), address.get_c(), address.get_d(), address.get_port(), handle);
            return std::make_tuple(address, buff);
        }
    }

//------------------------------------------------------------------------------    
    void UdpSocket::init(unsigned short port) 
    {
        // create socket
        handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle <= 0)
        {
            throw std::runtime_error("failed to create socket");
        }
        
        // bind to port
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons((unsigned short)port);
        int r = bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in));
        if (r < 0)
        {
            TRACE("Failed to bind port %d. (r = %d)", port, r);
            throw std::runtime_error("failed to bind socket");
        }
        else
        {
            TRACE("Successfully bound 0x%04X port %d.", handle, port);
        }
        
        // set non blocking mode
#if _WIN32
        DWORD nonBlocking = 1;
        if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
        {
            throw std::runtime_error("failed to set non-blocking socket");
        }
#else        
        int nonBlocking = 1;
        if (fcntl( handle, F_SETFL, O_NONBLOCK, nonBlocking ) == -1)
        {
            throw std::runtime_error("failed to set non-blocking socket");
        }
#endif
    }
}


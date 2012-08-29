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

#include <stdexcept>
#include <vector>

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
    }
    
//------------------------------------------------------------------------------
    std::tr1::tuple<Address, std::string > UdpSocket::recive()
    {
        std::string buff(get_max_packet_size(), 0);    
        sockaddr_in c_adr;
        int c_adr_len = sizeof(sockaddr_in);
        
        int received_bytes = recvfrom(handle, &buff[0], buff.size(), 0, (sockaddr*)&c_adr, &c_adr_len);
        
        if (received_bytes <= 0)
        {
            return std::tr1::make_tuple(Address(), std::string());
        }
        else
        {
            buff.resize(received_bytes);
            return std::tr1::make_tuple(Address(c_adr), buff);
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
        if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
        {
            throw std::runtime_error("failed to bind socket");
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


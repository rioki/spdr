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
    std::string get_error(int handle) 
    {
        #if _WIN32 
        switch (WSAGetLastError()) 
        {
            case WSANOTINITIALISED:
                return "A successful WSAStartup call must occur before using this function.";             
            case WSAENETDOWN:
                return "The network subsystem has failed.";                
            case WSAEACCES:            
                return "The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address.";                
            case WSAEINVAL:
                return "An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled.";
            case WSAEINTR:
                return "A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.";                
            case WSAEINPROGRESS:
                return "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
            case WSAEFAULT:
                return "The buf or to parameters are not part of the user address space, or the tolen parameter is too small.";
            case WSAENETRESET:
                return "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.";
            case WSAENOBUFS:            
                return "No buffer space is available.";
            case WSAENOTCONN:
                return "The socket is not connected.";
            case WSAENOTSOCK:
                return "The descriptor is not a socket.";
            case WSAEOPNOTSUPP:
                return "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.";
            case WSAESHUTDOWN:
                return "The socket has been shut down; it is not possible to sendto on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.";
            case WSAEWOULDBLOCK:
                return "The socket is marked as nonblocking and the requested operation would block.";
            case WSAEMSGSIZE:
                return "The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.";
            case WSAECONNABORTED:
                return "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.";
            case WSAECONNRESET:
                return "The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable.";
            case WSAEADDRNOTAVAIL:
                return "The remote address is not a valid address, for example, ADDR_ANY.";
            case WSAEAFNOSUPPORT:
                return "Addresses in the specified family cannot be used with this socket.";
            case WSAEDESTADDRREQ:
                return "A destination address is required.";
            case WSAENETUNREACH:
                return "The network cannot be reached from this host at this time.";
            case WSAEHOSTUNREACH:
                return "A socket operation was attempted to an unreachable host.";
            case WSAETIMEDOUT:
                return "The connection has been dropped, because of a network failure or because the system on the other end went down without notice.";
            default:
                return "failed to send packet";
        }
        #else            
        return "failed to send packet";
        #endif
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
        if (sent_bytes == SOCKET_ERROR)
        {
            throw std::runtime_error(get_error(handle));
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


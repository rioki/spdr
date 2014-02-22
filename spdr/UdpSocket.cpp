#include "UdpSocket.h"
 
#include <stdexcept>
 
#ifdef _WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
 
namespace spdr
{
    #define RECV_SIZE 512
 
    bool UdpSocket::init = false;
 
    UdpSocket::UdpSocket()
    : handle(0)
    {
        if (init == false)
        {
            #ifdef _WIN32
            WSADATA WsaData;
            int r = WSAStartup(MAKEWORD(2,2), &WsaData);
            if (r != NO_ERROR)
            {
                throw std::runtime_error("Failed to start WSA.");
            }
            #endif
            init = true;
        }
        
        handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle <= 0)
        {
            throw std::runtime_error("Failed to create socket.");
        }
        
        #ifdef _WIN32
        DWORD nonBlocking = 1;
        if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
        {
            throw std::runtime_error("Failed to set socket into non blocking mode.");
        }
        #else
        int nonBlocking = 1;
        if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
        {
            throw std::runtime_error("Failed to set socket into non blocking mode.");
        }
        #endif
    }
        
    UdpSocket::~UdpSocket()
    {
        #ifdef _WIN32
        closesocket(handle);
        #else
        close(handle);
        #endif
    }
        
    void UdpSocket::bind(unsigned short port)
    {
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
 
        if (::bind(handle, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0)
        {
            throw std::runtime_error("Failed to bind socket");
        }
    }
    
    void UdpSocket::send(const IpAddress& addr, const std::string& data)
    {
        if (data.empty())
        {
            throw std::invalid_argument("Trying to send a zero sized packet.");
        }
        if (data.size() > RECV_SIZE)
        {
            throw std::invalid_argument("Data exedes safe packet size.");
        }
        
        int r = sendto(handle, &data[0], data.size(), 0, addr, sizeof(sockaddr_in));
 
        if ((size_t)r != data.size())
        {
            throw std::runtime_error("Failed to send packet.");
        }
    }
    
    std::tuple<IpAddress, std::string> UdpSocket::recive()
    {
        IpAddress addr;
        socklen_t addr_size = sizeof(sockaddr_in);
        char buffer[RECV_SIZE];
        int r = recvfrom(handle, buffer, RECV_SIZE, 0, addr, &addr_size);
 
        /*if (r < 0) // error
        {
            throw std::runtime_error("Failed to receive packet.");
        }*/
        
        //if (r == 0) // no data
        if (r <= 0) 
        {
            return std::make_tuple(addr, std::string());
        }
        else
        {
            return std::make_tuple(addr, std::string(buffer, r));
        }
    }
}
#ifndef _SPDR_UDP_SCOKET_H_
#define _SPDR_UDP_SCOKET_H_
 
#include <string>
#include <tuple>
#include "IpAddress.h"
 
namespace spdr
{
    class UdpSocket
    {
    public:
        UdpSocket();
        
        ~UdpSocket();
        
        void bind(unsigned short port);
        
        void send(const IpAddress& addr, const std::string& data);
        
        std::tuple<IpAddress, std::string> recive();
        
    private:
        static bool init;
        int handle;
    
        UdpSocket(const UdpSocket&);
        const UdpSocket& operator = (const UdpSocket&);
    };
}
 
#endif
#ifndef _SPDR_IP_ADDRESS_H_
#define _SPDR_IP_ADDRESS_H_
 
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif
 
#include <string>
#include <iosfwd>
 
namespace spdr
{
    class IpAddress
    {
    public:
        IpAddress();
        
        IpAddress(const std::string& host, unsigned short port);
        
        IpAddress(const IpAddress& other);
        
        ~IpAddress();
        
        const IpAddress& operator = (const IpAddress& other);
        
        bool operator == (const IpAddress& other) const;
        
        bool operator != (const IpAddress& other) const;
        
        unsigned int get_host() const;
        
        unsigned short get_port() const;
        
        operator sockaddr* ();
        
        operator const sockaddr* () const;
        
    private:    
        sockaddr_in addr;
    };
    
    std::ostream& operator << (std::ostream& os, const IpAddress& addr);
}
 
#endif
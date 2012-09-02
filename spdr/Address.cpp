// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include "Address.h"

#include <iostream>

namespace spdr
{       
    Address::Address() 
    {
        c_obj.sin_family = AF_INET;
        c_obj.sin_addr.s_addr = INADDR_ANY;
        c_obj.sin_port = 0;
    }
    
    Address::Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port)
    {
        c_obj.sin_family = AF_INET;
        unsigned long ip_address = a << 24 | b << 16 | c << 8 | d;
        c_obj.sin_addr.s_addr = htonl(ip_address);
        c_obj.sin_port = htons(port);
    }

    Address::Address(sockaddr_in o)
    {
        c_obj = o;
    }
    
    void Address::set_a(unsigned char value)
    {
        unsigned int ip = get_ip();
        ip = (ip & 0x00FFFFFF) | (value << 24);
        set_ip(ip);
    }
    
    unsigned char Address::get_a() const
    {
        return (get_ip() & 0xFF000000) >> 24;
    }
    
    void Address::set_b(unsigned char value)
    {
        unsigned int ip = get_ip();
        ip = (ip & 0xFF00FFFF) | (value << 16);
        set_ip(ip);
    }

    unsigned char Address::get_b() const
    {
        return (get_ip() & 0x00FF0000) >> 16;
    }
    
    void Address::set_c(unsigned char value)
    {
        unsigned int ip = get_ip();
        ip = (ip & 0xFFFF00FF) | (value << 8);
        set_ip(ip);
    }

    unsigned char Address::get_c() const
    {
        return (get_ip() & 0x0000FF00) >> 8;
    }
    
    void Address::set_d(unsigned char value)
    {
        unsigned int ip = get_ip();
        ip = (ip & 0xFFFFFF00) | (value << 0);
        set_ip(ip);
    }

    unsigned char Address::get_d() const
    {
        return (get_ip() & 0x000000FF) >> 0;
    }
    
    void Address::set_ip(unsigned long value)
    {
        c_obj.sin_addr.s_addr = htonl(value);
    }

    unsigned long Address::get_ip() const
    {
        return ntohl(c_obj.sin_addr.s_addr);
    }
    
    void Address::set_port(unsigned short value)
    {
        c_obj.sin_port = htons(value);
    }

    unsigned short Address::get_port() const
    {
        return ntohs(c_obj.sin_port);
    }

    sockaddr_in Address::get_c_obj() const
    {
        return c_obj;
    }

    void Address::set_c_obj(sockaddr_in value)
    {
        c_obj = value;
    }

    bool operator == (const Address& a, const Address& b)
    {
        return a.get_ip() == b.get_ip() &&
               a.get_port() == b.get_port();
    }
    
    bool operator != (const Address& a, const Address& b)
    {
        return !(a == b);
    }

    std::ostream& operator << (std::ostream& os, const Address& a)
    {
        os << static_cast<unsigned int>(a.get_a()) << "."
           << static_cast<unsigned int>(a.get_b()) << "."
           << static_cast<unsigned int>(a.get_c()) << "."
           << static_cast<unsigned int>(a.get_d());
        
        if (a.get_port() != 0)
        {
            os << ":" << a.get_port();
        }
        
        return os;
    }
}
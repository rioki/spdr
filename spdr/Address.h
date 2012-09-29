// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifndef _SPDR_ADDRESS_H_
#define _SPDR_ADDRESS_H_

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <iosfwd>

namespace spdr
{
    /**
     * Network Address 
     **/
    class Address
    {
    public:
    
        static Address resolve(const std::string& host, unsigned short port);
    
        /**
         * Default Constructor
         **/
        Address();
        
        /**
         * Create a Address from four IP components. 
         **/
        Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port);
        
        /**
         * Create Address from underlying object. 
         **/
        Address(sockaddr_in c_obj);
        
        void set_a(unsigned char value);
        
        /**
         * Get A Component of the IP address.
         **/
        unsigned char get_a() const;
        
        void set_b(unsigned char value);
        
        /**
         * Get B Component of the IP address.
         **/
        unsigned char get_b() const;
        
        void set_c(unsigned char value);
        
        /**
         * Get C Component of the IP address.
         **/
        unsigned char get_c() const;
        
        void set_d(unsigned char value);
        
        /**
         * Get D Component of the IP address.
         **/
        unsigned char get_d() const;
        
        void set_ip(unsigned long value);
        
        /**
         * Get the entiere IP address.
         **/
        unsigned long get_ip() const;
        
        void set_port(unsigned short value);
        
        /**
         * Get network port.
         **/
        unsigned short get_port() const;
    
        /**
         * Get the internal object.
         **/
        sockaddr_in get_c_obj() const;
        
        /**
         * Set the internal object.
         **/
        void set_c_obj(sockaddr_in value);
        
    private:
        sockaddr_in c_obj;    
    };
    
    bool operator == (const Address& a, const Address& b);
    bool operator != (const Address& a, const Address& b);
    
    std::ostream& operator << (std::ostream& os, const Address& a);
}

#endif

// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include "UdpSocket.h"

#include <vector>
#include <string>
#include <tr1/tuple>

SUITE(UdpSocket)
{
//------------------------------------------------------------------------------
    TEST(max_packet_size)
    {
        spdr::UdpSocket socket;
        CHECK_EQUAL(512, socket.get_max_packet_size());
    }
    
//------------------------------------------------------------------------------
    TEST(send_and_recv)
    {
        spdr::UdpSocket socket1;
        spdr::UdpSocket socket2(1337);
        
        spdr::Address adr1(127,0,0,1,1337);
        std::string buff1 = "Hello Network.";
        socket1.send(adr1, buff1);
        
        spdr::Address adr2;
        std::string buff2;    
        while (buff2.empty())
        {
            std::tr1::tie(adr2, buff2) = socket2.recive();        
        }
        
        CHECK(buff1 == buff2);
    }
    
//------------------------------------------------------------------------------    
    /*TEST(resolve_host_name)
    {
        ib::IpAddress address = ib::IpAddress::resolve("localhost", 1337);
        CHECK_EQUAL(127, address.get_a());
        CHECK_EQUAL(0, address.get_b());
        CHECK_EQUAL(0, address.get_c());
        CHECK_EQUAL(1, address.get_d());
        CHECK_EQUAL(1337, address.get_port());
    }*/
}
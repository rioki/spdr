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
        std::string value = "Hello Network.";
        std::vector<char> buff1(value.begin(), value.end());
        socket1.send(adr1, buff1);
        
        spdr::Address adr2;
        std::vector<char> buff2;    
        while (buff2.empty())
        {
            std::tr1::tie(adr2, buff2) = socket2.recive();        
        }
        
        CHECK(buff1 == buff2);
    }
}
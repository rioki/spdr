// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include "Address.h"

SUITE(Address)
{
//------------------------------------------------------------------------------
    TEST(default_constructoble)
    {
        spdr::Address adr;
        
        CHECK_EQUAL(0, adr.get_a());
        CHECK_EQUAL(0, adr.get_b());
        CHECK_EQUAL(0, adr.get_c());
        CHECK_EQUAL(0, adr.get_d());
        CHECK_EQUAL(0, adr.get_port());
        
    }
    
//------------------------------------------------------------------------------
    TEST(from_ip)
    {
        spdr::Address adr(192, 168, 1, 101, 1337);
        
        // NOTE: we cast the chars to into so they are interpretated as int...
        CHECK_EQUAL(192,  static_cast<unsigned short>(adr.get_a()));
        CHECK_EQUAL(168,  static_cast<unsigned short>(adr.get_b()));
        CHECK_EQUAL(1,    static_cast<unsigned short>(adr.get_c()));
        CHECK_EQUAL(101,  static_cast<unsigned short>(adr.get_d()));
        CHECK_EQUAL(1337, adr.get_port());
    }
        
//------------------------------------------------------------------------------    
    TEST(resolve_host_name)
    {
        spdr::Address address = spdr::Address::resolve("localhost", 1337);
        CHECK_EQUAL(127, address.get_a());
        CHECK_EQUAL(0, address.get_b());
        CHECK_EQUAL(0, address.get_c());
        CHECK_EQUAL(1, address.get_d());
        CHECK_EQUAL(1337, address.get_port());
    }
        
//------------------------------------------------------------------------------    
    TEST(ip_from_text)
    {
        spdr::Address address = spdr::Address::resolve("127.0.0.1", 1337);
        CHECK_EQUAL(127, address.get_a());
        CHECK_EQUAL(0, address.get_b());
        CHECK_EQUAL(0, address.get_c());
        CHECK_EQUAL(1, address.get_d());
        CHECK_EQUAL(1337, address.get_port());
    }
}

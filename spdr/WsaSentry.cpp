// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifdef _WIN32

#include "WsaSentry.h"

#include <winsock2.h>
#include <iostream>

namespace spdr
{
//------------------------------------------------------------------------------
    WsaSentry::WsaSentry() 
    {
        WSADATA WsaData;
        int result = WSAStartup(MAKEWORD(2,2), &WsaData);
        if (result != NO_ERROR)
        {
            std::cerr << "Failed to start WSA" << std::endl;
        }        
    }
    
//------------------------------------------------------------------------------
    WsaSentry::~WsaSentry()
    {
        WSACleanup();
    }
    
    WsaSentry wsa_sentry;
}

#endif

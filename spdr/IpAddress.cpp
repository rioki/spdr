// spdr
// Copyright 2011-2023 Sean Farrell
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"
#include "IpAddress.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace spdr
{
    IpAddress::IpAddress()
    {
        std::memset(&addr, 0, sizeof(sockaddr_in));
    }

    IpAddress::IpAddress(const std::string& host, unsigned short port)
    {
        addr.sin_family = AF_INET;
        if (host.empty())
        {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            addr.sin_addr.s_addr = inet_addr(host.c_str());
            if (addr.sin_addr.s_addr == INADDR_NONE)
            {
                struct hostent* hp = gethostbyname(host.c_str());
                if (hp)
                {
                    memcpy(&addr.sin_addr.s_addr, hp->h_addr, hp->h_length);
                }
                else
                {
                    throw std::runtime_error("Failed to resolve host " + host);
                }
            }
        }
        addr.sin_port = htons(port);
    }

    IpAddress::IpAddress(const IpAddress& other)
    {
        std::memcpy(&addr, &other.addr, sizeof(sockaddr_in));
    }

    IpAddress::~IpAddress() {}

    const IpAddress& IpAddress::operator = (const IpAddress& other)
    {
        if (this != &other)
        {
            std::memcpy(&addr, &other.addr, sizeof(sockaddr_in));
        }
        return *this;
    }

    bool IpAddress::operator == (const IpAddress& other) const
    {
        return get_host() == other.get_host() &&
               get_port() == other.get_port();
    }

    bool IpAddress::operator != (const IpAddress& other) const
    {
        return !(*this == other);
    }

    unsigned int IpAddress::get_host() const
    {
        return ntohl(addr.sin_addr.s_addr);
    }

    unsigned short IpAddress::get_port() const
    {
        return ntohs(addr.sin_port);
    }

    IpAddress::operator sockaddr* ()
    {
        return reinterpret_cast<sockaddr*>(&addr);
    }

    IpAddress::operator const sockaddr* () const
    {
        return reinterpret_cast<const sockaddr*>(&addr);
    }

    std::ostream& operator << (std::ostream& os, const IpAddress& addr)
    {
        unsigned int host = addr.get_host();
        unsigned short port = addr.get_port();

        unsigned int a = (host & 0xFF000000) >> 24;
        unsigned int b = (host & 0x00FF0000) >> 16;
        unsigned int c = (host & 0x0000FF00) >>  8;
        unsigned int d = (host & 0x000000FF) >>  0;

        os << a << "." << b << "." << c << "." << d << ":" << port;

        return os;
    }
}
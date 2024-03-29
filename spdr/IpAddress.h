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

#pragma once
#include "config.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <string>
#include <iosfwd>

namespace spdr
{
    //! IP Address
    class SPDR_EXPORT IpAddress
    {
    public:
        //! Create null IP address (0.0.0.0:0)
        IpAddress() noexcept;

        //! Create IP addres from stirng address or host name and port.
        //!
        //! @param host either string representation of address or host name
        //! @param port port to connect to
        IpAddress(const std::string& host, unsigned short port);

        //! Copy consturctor
        IpAddress(const IpAddress& other) noexcept;

        ~IpAddress() = default;

        //! Assignment operator
        const IpAddress& operator = (const IpAddress& other) noexcept;

        //! Equal comparision operator
        bool operator == (const IpAddress& other) const noexcept;

        //! Get the host address.
        unsigned int get_host() const noexcept;

        //! Get the port.
        unsigned short get_port() const noexcept;

        //! Get undelying C object
        //!
        //! @{
        sockaddr* c_obj() noexcept;
        const sockaddr* c_obj() const noexcept;
        //! @}

    private:
        sockaddr_in addr;
    };

    //! Write IP address to stream.
    SPDR_EXPORT std::ostream& operator << (std::ostream& os, const IpAddress& addr) noexcept;
}

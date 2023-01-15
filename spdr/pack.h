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

#include <iosfwd>
#include <tuple>
#include <utility>
#include <vector>
#include <string>

namespace spdr
{
    SPDR_EXPORT void pack(std::ostream& os, bool value);
    SPDR_EXPORT void pack(std::ostream& os, char value);
    SPDR_EXPORT void pack(std::ostream& os, unsigned char value);
    SPDR_EXPORT void pack(std::ostream& os, short value);
    SPDR_EXPORT void pack(std::ostream& os, unsigned short value);
    SPDR_EXPORT void pack(std::ostream& os, int value);
    SPDR_EXPORT void pack(std::ostream& os, unsigned int value);
    SPDR_EXPORT void pack(std::ostream& os, long value);
    SPDR_EXPORT void pack(std::ostream& os, unsigned long value);
    SPDR_EXPORT void pack(std::ostream& os, long long value);
    SPDR_EXPORT void pack(std::ostream& os, unsigned long long value);
    SPDR_EXPORT void pack(std::ostream& os, float value);
    SPDR_EXPORT void pack(std::ostream& os, double value);
    SPDR_EXPORT void pack(std::ostream& os, const std::string& value);

    template <typename T>
    void pack(std::ostream& os, const std::vector<T>& value)
    {
        pack(os, value.size());
        for (unsigned int i = 0; i < value.size(); i++)
        {
            pack(os, value[i]);
        }
    }

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I == sizeof...(Tp), void>::type
    pack(std::ostream& os, const std::tuple<Tp...>& t) { }

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I < sizeof...(Tp), void>::type
    pack(std::ostream& os, const std::tuple<Tp...>& t)
    {
        pack(os, std::get<I>(t));
        pack<I + 1, Tp...>(os, t);
    }

    SPDR_EXPORT void unpack(std::istream& is, bool& value);
    SPDR_EXPORT void unpack(std::istream& is, char& value);
    SPDR_EXPORT void unpack(std::istream& is, unsigned char& value);
    SPDR_EXPORT void unpack(std::istream& is, short& value);
    SPDR_EXPORT void unpack(std::istream& is, unsigned short& value);
    SPDR_EXPORT void unpack(std::istream& is, int& value);
    SPDR_EXPORT void unpack(std::istream& is, unsigned int& value);
    SPDR_EXPORT void unpack(std::istream& is, long& value);
    SPDR_EXPORT void unpack(std::istream& is, unsigned long& value);
    SPDR_EXPORT void unpack(std::istream& is, long long& value);
    SPDR_EXPORT void unpack(std::istream& is, unsigned long long& value);
    SPDR_EXPORT void unpack(std::istream& is, float& value);
    SPDR_EXPORT void unpack(std::istream& is, double& value);
    SPDR_EXPORT void unpack(std::istream& is, std::string& value);

    template <typename T>
    void unpack(std::istream& is, std::vector<T>& value)
    {
        size_t size;
        unpack(is, size);

        value.resize(size);
        for (unsigned int i = 0; i < value.size(); i++)
        {
            unpack(is, value[i]);
        }
    }

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I == sizeof...(Tp), void>::type
    unpack(std::istream& is, std::tuple<Tp...>& t) { }

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I < sizeof...(Tp), void>::type
    unpack(std::istream& is, std::tuple<Tp...>& t)
    {
        unpack(is, std::get<I>(t));
        unpack<I + 1, Tp...>(is, t);
    }
}

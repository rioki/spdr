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
#include "pack.h"

#include <iostream>
#include <cstring>

namespace spdr
{
    void pack(std::ostream& os, bool value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, char value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, unsigned char value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, short value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, unsigned short value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, int value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, unsigned int value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, long value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, unsigned long value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, long long value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, unsigned long long value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, float value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, double value) noexcept
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void pack(std::ostream& os, const std::string& value) noexcept
    {
        unsigned int size = static_cast<unsigned int>(value.size());
        pack(os, size);
        if (size != 0)
        {
            os.write(&value[0], size * sizeof(char));
        }
    }

    void unpack(std::istream& is, bool& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, char& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, unsigned char& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, short& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, unsigned short& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, int& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, unsigned int& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, long& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, unsigned long& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, long long& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, unsigned long long& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, float& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, double& value) noexcept
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }

    void unpack(std::istream& is, std::string& value) noexcept
    {
        unsigned int size = 0;
        unpack(is, size);
        if (size != 0)
        {
            value = std::string(size, '\0');
            is.read(&value[0], size * sizeof(char));
        }
        else
        {
            value = std::string();
        }
    }
}


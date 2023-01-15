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

TEST(pack, pod_types)
{
    bool               bool_value   = true;
    char               char_value   = 'a';
    unsigned char      uchar_value  = 42u;
    short              short_value  = 43;
    unsigned short     ushort_value = 44u;
    long               long_value   = 45;
    unsigned long      ulong_value  = 46u;
    long long          llong_value  = 47;
    unsigned long long ullong_value = 48u;
    float              float_value  = 4.9f;
    double             double_value = 5.0;
    std::string        string_value = "Hello";

    std::stringstream buff;
    spdr::pack(buff, bool_value);
    spdr::pack(buff, char_value);
    spdr::pack(buff, uchar_value);
    spdr::pack(buff, short_value);
    spdr::pack(buff, ushort_value);
    spdr::pack(buff, long_value);
    spdr::pack(buff, ulong_value);
    spdr::pack(buff, llong_value);
    spdr::pack(buff, ullong_value);
    spdr::pack(buff, float_value);
    spdr::pack(buff, double_value);
    spdr::pack(buff, string_value);

    bool               bool_result  = false;
    char               char_result   = 0;
    unsigned char      uchar_result  = 0;
    short              short_result  = 0;
    unsigned short     ushort_result = 0;
    long               long_result   = 0;
    unsigned long      ulong_result  = 0;
    long long          llong_result  = 0;
    unsigned long long ullong_result = 0;
    float              float_result  = 0.0f;
    double             double_result = 0.0;
    std::string        string_result;

    spdr::unpack(buff, bool_result);
    spdr::unpack(buff, char_result);
    spdr::unpack(buff, uchar_result);
    spdr::unpack(buff, short_result);
    spdr::unpack(buff, ushort_result);
    spdr::unpack(buff, long_result);
    spdr::unpack(buff, ulong_result);
    spdr::unpack(buff, llong_result);
    spdr::unpack(buff, ullong_result);
    spdr::unpack(buff, float_result);
    spdr::unpack(buff, double_result);
    spdr::unpack(buff, string_result);


    EXPECT_EQ(bool_value  , bool_result  );
    EXPECT_EQ(char_value  , char_result  );
    EXPECT_EQ(uchar_value , uchar_result );
    EXPECT_EQ(short_value , short_result );
    EXPECT_EQ(ushort_value, ushort_result);
    EXPECT_EQ(long_value  , long_result  );
    EXPECT_EQ(ulong_value , ulong_result );
    EXPECT_EQ(llong_value , llong_result );
    EXPECT_EQ(ullong_value, ullong_result);
    EXPECT_EQ(float_value , float_result );
    EXPECT_EQ(double_value, double_result);
    EXPECT_EQ(string_value, string_result);
}

TEST(pack, vector)
{
    auto value  = std::vector<int>{12u, 13u, 14u, 15u};
    auto result = std::vector<int>{};

    auto buff = std::stringstream{};
    spdr::pack(buff, value);
    spdr::unpack(buff, result);

    EXPECT_EQ(value, result);
}

TEST(pack, touple)
{
    auto value  = std::tuple<int, int, float>{12u, 13u, 14.5f};
    auto result = std::tuple<int, int, float>{};

    auto buff = std::stringstream{};
    spdr::pack(buff, value);
    spdr::unpack(buff, result);

    EXPECT_EQ(value, result);
}

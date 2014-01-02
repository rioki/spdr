
#ifndef _SPDR_PACK_H_
#define _SPDR_PACK_H_

#include <iosfwd>

namespace spdr
{    
    void pack(std::ostream& os, bool value);
    void pack(std::ostream& os, char value);
    void pack(std::ostream& os, unsigned char value);
    void pack(std::ostream& os, short value);
    void pack(std::ostream& os, unsigned short value);
    void pack(std::ostream& os, int value);
    void pack(std::ostream& os, unsigned int value);
    void pack(std::ostream& os, long value);
    void pack(std::ostream& os, unsigned long value);
    void pack(std::ostream& os, long long value);
    void pack(std::ostream& os, unsigned long long value);    
    void pack(std::ostream& os, float value);
    void pack(std::ostream& os, double value);
    void pack(std::ostream& os, const char* value);
    void pack(std::ostream& os, const std::string& value);
    
    void unpack(std::istream& is, bool& value);
    void unpack(std::istream& is, char& value);
    void unpack(std::istream& is, unsigned char& value);
    void unpack(std::istream& is, short& value);
    void unpack(std::istream& is, unsigned short& value);
    void unpack(std::istream& is, int& value);
    void unpack(std::istream& is, unsigned int& value);
    void unpack(std::istream& is, long& value);
    void unpack(std::istream& is, unsigned long& value);
    void unpack(std::istream& is, long long& value);
    void unpack(std::istream& is, unsigned long long& value);    
    void unpack(std::istream& is, float& value);
    void unpack(std::istream& is, double& value);
    void unpack(std::istream& is, std::string& value);
}

#endif
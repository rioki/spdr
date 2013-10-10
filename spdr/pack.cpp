
#include "pack.h"

#include <iostream>
#include <cstring>

namespace spdr
{
    void pack(std::ostream& os, bool value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
   
    void pack(std::ostream& os, char value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, unsigned char value)    
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, short value)    
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, unsigned short value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, int value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, unsigned int value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, long value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, unsigned long value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, long long value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, unsigned long long value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, float value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, double value)
    {
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    
    void pack(std::ostream& os, const char* value)
    {
        unsigned int size = strlen(value);
        pack(os, size);
        if (size != 0)
        {
            os.write(value, size * sizeof(char));
        }
    }
    
    void pack(std::ostream& os, const std::string& value)
    {
        unsigned int size = value.size();
        pack(os, size);
        if (size != 0)
        {
            os.write(&value[0], size * sizeof(char));
        }
    }
    
    void unpack(std::istream& is, bool& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, char& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, unsigned char& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, short& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, unsigned short& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, int& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, unsigned int& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, long& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, unsigned long& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, long long& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, unsigned long long& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, float& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, double& value)
    {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
    }
    
    void unpack(std::istream& is, std::string& value)
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


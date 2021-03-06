
#ifndef _SPDR_PACK_H_
#define _SPDR_PACK_H_

#include <iosfwd>
#include <tuple>
#include <utility> 
#include <vector>

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

#endif

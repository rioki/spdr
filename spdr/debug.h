
#ifndef _SPDR_DEBUG_H_
#define _SPDR_DEBUG_H_

#ifdef DEBUG

namespace spdr
{
    void trace(const char* msg, ...);
}

#define TRACE(MSG, ...) ::spdr::trace(MSG, __VA_ARGS__) 
#else
#define TRACE(MSG, ...) do {} while(false)
#endif 

#endif

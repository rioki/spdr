
#include "debug.h"

#ifdef DEBUG

#include <stdarg.h>
#include <stdio.h>

namespace spdr
{
    void trace(const char* msg, ...)
    {
        const unsigned int size = 1024;
        char buff[size];
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, size, msg, args);
        va_end(args);
        
        fprintf(stderr, "SPDR %s\n", buff);
    }
}

#endif  



#include "debug.h"

#ifdef DEBUG

#include <stdarg.h>
#include <stdio.h>

#include <c9y/Mutex.h>
#include <c9y/Lock.h>

namespace spdr
{
    c9y::Mutex mutex;
    
    void trace(const char* msg, ...)
    {
        const unsigned int size = 1024;
        char buff[size];
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, size, msg, args);
        va_end(args);
        
        c9y::Lock<c9y::Mutex> lock(mutex);
        fprintf(stderr, "SPDR %s\n", buff);
        fflush(stderr);
    }
}

#endif  


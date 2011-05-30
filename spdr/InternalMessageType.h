// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _INTERNAL_MESSAGE_TYPE_H_
#define _INTERNAL_MESSAGE_TYPE_H_

namespace spdr
{
    /**
     * Types of internal messages.
     **/
    enum InternalMessageType
    {
        CONNECT_MESSAGE = 1,
        ACCEPT_MESSAGE = 2,        
        REJECT_MESSAGE = 3,
        KEEP_ALIVE_MESSAGE = 4
    };
}

#endif

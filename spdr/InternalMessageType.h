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
        CONNECT = 1,
        CONNECTION_ACCEPTED = 2,        
        CONNECTION_REJECTED = 3,
    };
}

#endif

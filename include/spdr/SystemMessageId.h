// Iced Blue
// Copyright 2011 Sean Farrell

#ifndef _IBNET_SYSTEM_MESSAGE_ID_H_
#define _IBNET_SYSTEM_MESSAGE_ID_H_

namespace spdr
{
    enum SystemMessageId
    {
        CONNECT_MESSAGE = 0,
        CONNECTION_ACCEPTED_MESSAGE = 1,
        CONNECTION_REJECTED_MESSAGE = 2,
        DISCONNECT_MESSAGE = 3,
        KEEP_ALIVE_MESSAGE = 4        
    };
}

#endif

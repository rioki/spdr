// Iced Blue
// Copyright 2011 Sean Farrell

#ifndef _IBNET_CONNECT_MESSAGE_H_
#define _IBNET_CONNECT_MESSAGE_H_

#include "Message.h"

namespace spdr
{
    class KeepAliveMessage : public Message
    {
    public:
    
        KeepAliveMessage();
        
        virtual ~KeepAliveMessage();
        
        virtual unsigned int get_id() const;
        
        virtual void pack(std::ostream& os) const;
        
        virtual void unpack(std::istream& is);
        
    private:
    };
}

#endif

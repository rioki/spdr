// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#ifndef _SPDR_CONNECT_MESSAGE_H_
#define _SPDR_CONNECT_MESSAGE_H_

#include "Message.h"

namespace spdr
{
    enum
    {
        KEEP_ALIVE_MESSAGE = 0
    };   
    
    class KeepAliveMessage : public Message
    {
    public:
    
        KeepAliveMessage();
        
        virtual ~KeepAliveMessage();
        
        virtual unsigned int get_id() const;
        
        virtual bool is_reliable() const;
        
        virtual void pack(std::ostream& os) const;
        
        virtual void unpack(std::istream& is);
        
    private:
    };
}

#endif

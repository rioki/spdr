// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_MESSAGE_FACTORY_H_
#define _SPDR_MESSAGE_FACTORY_H_

#include <map>
#include <tr1/memory>
#include <c9y/Mutex.h>

#include "Message.h"

namespace spdr
{
    class MessageFactory
    {
    public:
        MessageFactory();
        
        ~MessageFactory();
        
        void add(std::tr1::shared_ptr<Message> message);
        
        std::tr1::shared_ptr<Message> create(unsigned int type) const;
    
    private:
        mutable c9y::Mutex mutex;
        std::map<unsigned int, std::tr1::shared_ptr<Message> > prototypes;
    
        MessageFactory(const MessageFactory&);        
        const MessageFactory& operator = (const MessageFactory&);
    };
}

#endif

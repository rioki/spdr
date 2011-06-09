// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "MessageFactory.h"

#include <sanity/check.h>
#include <c9y/Lock.h>

using namespace std;
using namespace std::tr1;
using namespace c9y;

namespace spdr
{
//------------------------------------------------------------------------------
    MessageFactory::MessageFactory() {}

//------------------------------------------------------------------------------    
    MessageFactory::~MessageFactory() {}

//------------------------------------------------------------------------------    
    void MessageFactory::add(shared_ptr<Message> message)
    {
        CHECK_ARGUMENT(message);
        
        Lock<Mutex> lock(mutex);
        ENSURE(prototypes.find(message->get_type()) == prototypes.end());
        
        prototypes[message->get_type()] = message;
    }

//------------------------------------------------------------------------------    
    shared_ptr<Message> MessageFactory::create(unsigned int type) const
    {
        Lock<Mutex> lock(mutex);
        
        map<unsigned int, shared_ptr<Message> >::const_iterator iter;
        iter = prototypes.find(type);
        if (iter != prototypes.end())
        {
            return iter->second;
        }
        else
        {
            throw std::logic_error("Unknown message type.");
        }
    }
}

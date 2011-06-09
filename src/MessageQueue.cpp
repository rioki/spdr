// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "MessageQueue.h"

#include <c9y/Lock.h>

namespace spdr
{
//------------------------------------------------------------------------------
    MessageQueue::MessageQueue() {}

//------------------------------------------------------------------------------    
    MessageQueue::~MessageQueue() {}
    
//------------------------------------------------------------------------------    
    void MessageQueue::push(std::tr1::shared_ptr<Message> message) 
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        queue.push(message);
    }
    
//------------------------------------------------------------------------------    
    std::tr1::shared_ptr<Message> MessageQueue::pop() 
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        if (!queue.empty())
        {
            std::tr1::shared_ptr<Message> message = queue.front();
            queue.pop();
            return message;
        }
        else
        {
            return std::tr1::shared_ptr<Message>();
        }
    }
}

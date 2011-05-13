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
    void MessageQueue::push(MessagePtr message) 
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        queue.push(message);
    }
    
//------------------------------------------------------------------------------    
    MessagePtr MessageQueue::pop() 
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        if (!queue.empty())
        {
            MessagePtr message = queue.front();
            queue.pop();
            return message;
        }
        else
        {
            return MessagePtr();
        }
    }
}

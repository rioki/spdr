// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_MESSAGE_QUEUE_H_
#define _SPDR_MESSAGE_QUEUE_H_

#include <queue>
#include <c9y/Mutex.h>

#include "Message.h"

namespace spdr
{
    /**
     * Queue of Messages
     **/
    class MessageQueue
    {
    public:
        /**
         * Constructor 
         **/
        MessageQueue();
        
        /**
         * Destructor 
         **/
        ~MessageQueue();
        
        /**
         * Enqueue a message. 
         * 
         * @param message the message to enqueue
         **/
        void push(MessagePtr message);
        
        /**
         * Dequeue a message.
         *
         * @return The front message or empty pointer if no messages are present.
         **/
        MessagePtr pop();
        
    private:
        c9y::Mutex mutex;
        std::queue<MessagePtr> queue;
    
        MessageQueue(const MessageQueue&);
        const MessageQueue& operator = (const MessageQueue&);
    };
}

#endif

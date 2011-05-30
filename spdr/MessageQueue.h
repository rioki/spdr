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
        void push(std::tr1::shared_ptr<Message> message);
        
        /**
         * Dequeue a message.
         *
         * @return The front message or empty pointer if no messages are present.
         **/
        std::tr1::shared_ptr<Message> pop();
        
    private:
        c9y::Mutex mutex;
        std::queue<std::tr1::shared_ptr<Message> > queue;
    
        MessageQueue(const MessageQueue&);
        const MessageQueue& operator = (const MessageQueue&);
    };
}

#endif

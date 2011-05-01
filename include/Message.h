// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_MESSAGE_H_
#define _SPDR_MESSAGE_H_

#include <tr1/memory>
#include <vector>

#include "Node.h"

namespace spdr
{
    /**
     * Message
     **/
    class Message
    {
    public:
    
        Message(NodePtr to);
        
        Message(NodePtr to, NodePtr from);
        
        virtual ~Message();
        
        NodePtr get_from() const;
        
        NodePtr get_to() const;
        
        virtual unsigned int get_type() const = 0;
        
        virtual std::vector<char> get_payload() const = 0;        
    
    private:
        NodePtr from;
        NodePtr to;
        
        Message(const Message&);
        const Message& operator = (const Message&);
    
    friend class Network;
    };

    typedef std::tr1::shared_ptr<Message> MessagePtr;
    
}

#endif

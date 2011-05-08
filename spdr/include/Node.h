// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NODE_H_
#define _SPDR_NODE_H_

#include <tr1/memory>

#include "Address.h"

namespace spdr
{
    class Node
    {
    public:
        
        const Address& get_address() const;
    
    private:
        Address address;
        
        Node();
        
        Node(const Address& address);        
        
    friend class Network;
    };
    
    typedef std::tr1::shared_ptr<Node> NodePtr;
}

#endif

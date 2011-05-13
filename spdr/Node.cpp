// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Node.h"

#include <cassert>
#include <iostream>

namespace spdr
{
//------------------------------------------------------------------------------
    Node::State Node::get_state() const
    {
        return state;
    }
    
//------------------------------------------------------------------------------    
    const Address& Node::get_address() const
    {
        return address;
    }

//------------------------------------------------------------------------------        
    unsigned int Node::get_last_message_recived() const
    {
        return last_message_recived;
    }    
    
//------------------------------------------------------------------------------    
    Node::Node()
    : state(CONNECTING), last_message_recived(0) {}

//------------------------------------------------------------------------------    
    Node::Node(const Address& a)
    : state(CONNECTING), address(a), last_message_recived(0) {}

//------------------------------------------------------------------------------    
    void Node::set_state(State value)
    {
        state = value;
    }    

//------------------------------------------------------------------------------    
    std::ostream& operator << (std::ostream& os, Node::State state)
    {
        switch (state)
        {
            case Node::CONNECTING:
                os << "CONNECTING";
                break;
            case Node::CONNECTED:
                os << "CONNECTED";
                break;
            case Node::TIMEOUT:
                os << "TIMEOUT";
                break;
            case Node::DISCONNECTED:
                os << "DISCONNECTED";
                break;
            default:
                assert(false);
                os << static_cast<unsigned int>(state);
                break;
        }
        return os;
    }
}

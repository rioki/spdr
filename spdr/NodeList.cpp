// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "NodeList.h"

#include <cassert>
#include <algorithm>
#include <c9y/Lock.h>

namespace spdr
{
    unsigned int get_time();
    
//------------------------------------------------------------------------------
    NodeList::NodeList() {}

//------------------------------------------------------------------------------    
    NodeList::~NodeList() {}

//------------------------------------------------------------------------------    
    void NodeList::add(NodePtr node)
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        
        nodes.push_back(node);
    }

//------------------------------------------------------------------------------    
    void NodeList::remove(NodePtr node)
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        
        std::vector<NodePtr>::iterator iter;
        iter = std::find(nodes.begin(), nodes.end(), node);
        
        assert(iter != nodes.end());
        nodes.erase(iter);
    }

//------------------------------------------------------------------------------    
    struct address_equals
    {
        const Address& address;
        
        address_equals(const Address& a)
        : address(a) {}
        
        bool operator () (NodePtr node)
        {
            return address == node->get_address();
        }
    };

//------------------------------------------------------------------------------
    NodePtr NodeList::get_node_by_address(const Address& address)
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        
        std::vector<NodePtr>::iterator iter;
        iter = std::find_if(nodes.begin(), nodes.end(), address_equals(address));
        if (iter != nodes.end())
        {
            return *iter;
        }
        else
        {
            return NodePtr();
        }
    }

//------------------------------------------------------------------------------    
    std::vector<NodePtr> NodeList::get_timout_nodes()
    {
        std::vector<NodePtr> result;
        
        unsigned int now = get_time();
        
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            if (now - nodes[i]->get_last_message_recived() > 500)
            {
                result.push_back(nodes[i]);
            }
        }
        
        return result;
    }
}

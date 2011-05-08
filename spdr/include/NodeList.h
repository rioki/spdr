// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NODE_LIST_H_
#define _SPDR_NODE_LIST_H_

#include <vector>
#include <c9y/Mutex.h>

#include "Node.h"
#include "Address.h"

namespace spdr
{
    /**
     * List of Nodes
     **/
    class NodeList
    {
    public:
        /**
         * Constructor 
         **/
        NodeList();
        
        /**
         * Destructor 
         **/
        ~NodeList();
        
        /**
         * Add a node to the list. 
         **/
        void add(NodePtr node);
        
        /**
         * Remove a node to the list. 
         **/
        void remove(NodePtr node);
        
        /**
         * Get the node by address.
         *
         * @return returns empy pointer if none was found
         **/
        NodePtr get_node_by_address(const Address& address);
    
    private:
        c9y::Mutex mutex; // RW-Mutex?
        std::vector<NodePtr> nodes;
        
        NodeList(const NodeList&);
        const NodeList& operator = (const NodeList&);
    };
    
    typedef std::tr1::shared_ptr<Node> NodePtr;
}

#endif
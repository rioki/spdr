// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_NODE_LIST_H_
#define _SPDR_NODE_LIST_H_

#include <vector>
#include <c9y/Mutex.h>
#include <c9y/Lock.h>

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
        
        /**
         * Get all nodes that are timedout. 
         **/
        std::vector<NodePtr> get_timout_nodes();
        
        template <typename Pred>
        std::vector<NodePtr> get_nodes(Pred predicate)
        {
            c9y::Lock<c9y::Mutex> lock(mutex);
            std::vector<NodePtr> result;
            for (unsigned int i = 0; i < nodes.size(); i++)
            {
                if (predicate(nodes[i]))
                {
                    result.push_back(nodes[i]);
                }
            }
            return result;
        }
    
    private:
        c9y::Mutex mutex; // RW-Mutex?
        std::vector<NodePtr> nodes;
        
        NodeList(const NodeList&);
        const NodeList& operator = (const NodeList&);
    };
    
    typedef std::tr1::shared_ptr<Node> NodePtr;
}

#endif
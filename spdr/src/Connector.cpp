// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Connector.h"

#include <stdexcept>
#include <sigc++/sigc++.h>

#include "InternalMessageType.h"
#include "ConnectMessage.h"

namespace spdr
{
//------------------------------------------------------------------------------
    Connector::Connector(Network& n)
    : network(n) {}
        
//------------------------------------------------------------------------------
    NodePtr Connector::connect(const Address& address)
    {
        connect_condition.clear();
        
        network.internal_message_recived.connect(sigc::mem_fun(this, &Connector::on_message_recived));
        
        node = network.create_node(address);        
        
        // TODO: make protocoll version a define
        MessagePtr conect_message(new ConnectMessage(node, 1));
        network.send(conect_message);
        
        connect_condition.wait();
        
        // error cond?
        if (node)
        {
            return node;
        }
        else
        {
            throw std::runtime_error("Connection failed.");
        }
    }

//------------------------------------------------------------------------------    
    void Connector::on_message_recived(MessagePtr message)
    {
        if (message->get_from() == node)
        {
            switch (message->get_type())
            {
                case CONNECTION_ACCEPTED:
                {
                    connect_condition.signal();
                }                
                case CONNECTION_REJECTED:
                {
                    network.remove_node(node);
                    connect_condition.signal();
                }
            }        
        }
    }
}

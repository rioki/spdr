// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Connector.h"

#include <stdexcept>
#include <sigc++/sigc++.h>
#include <c9y/Lock.h>

#include "InternalMessageType.h"
#include "ConnectMessage.h"

namespace spdr
{
//------------------------------------------------------------------------------
    Connector::Connector(Network& n)
    : network(n), 
      timer(100, sigc::mem_fun(this, &Connector::on_timeout)), 
      retry_count(0),
      state(INIT) {}
        
//------------------------------------------------------------------------------
    NodePtr Connector::connect(const Address& address)
    {
        connect_condition.clear();
        state = WAITING;
        
        network.internal_message_recived.connect(sigc::mem_fun(this, &Connector::on_message_recived));
        
        node = network.create_node(address);        
        
        // TODO: make protocoll version a define
        MessagePtr conect_message(new ConnectMessage(node, 1));
        network.send(conect_message);
        
        timer.start();
        connect_condition.wait();
        
        {
            c9y::Lock<c9y::Mutex> lock(mutex);        
            if (state == CONNECTED)
            {
                return node;
            }
            else
            {
                throw std::runtime_error("Connection failed.");
            }
        }
    }

//------------------------------------------------------------------------------    
    void Connector::on_message_recived(MessagePtr message)
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        
        if (message->get_from() == node)
        {
            switch (message->get_type())
            {
                case CONNECTION_ACCEPTED:
                {
                    if (state == WAITING)
                    {
                        state = CONNECTED;
                        connect_condition.signal();
                    }
                }                
                case CONNECTION_REJECTED:
                {
                    if (state == WAITING)
                    {
                        network.remove_node(node);
                        state = FAILED;
                        connect_condition.signal();
                    }
                }
            }        
        }
    }

//------------------------------------------------------------------------------    
    void Connector::on_timeout()
    {
        c9y::Lock<c9y::Mutex> lock(mutex);
        
        if (retry_count > 5)
        {
            if (state == WAITING)
            {
                network.remove_node(node);
                state = FAILED;
                connect_condition.signal();                
            }
        }
        
        retry_count++;
        MessagePtr conect_message(new ConnectMessage(node, 1));
        network.send(conect_message);
    }
}

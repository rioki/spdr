// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_CONECTOR_H_
#define _SPDR_CONECTOR_H_

#include "Node.h"
#include "Network.h"
#include "Address.h"

#include <c9y/Condition.h>
#include <sigc++/trackable.h>

namespace spdr
{
    /**
     * Helper class that handles connection.
     **/
    class Connector : public sigc::trackable
    {
    public:
    
        Connector(Network& network);
        
        NodePtr connect(const Address& address);
        
    private:        
        Network& network;
        NodePtr node;
        c9y::Condition connect_condition;
        
        void on_message_recived(MessagePtr message);
    };
}

#endif

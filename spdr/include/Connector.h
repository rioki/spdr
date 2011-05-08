// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_CONECTOR_H_
#define _SPDR_CONECTOR_H_

#include "Node.h"
#include "Network.h"
#include "Address.h"

#include <sigc++/trackable.h>
#include <c9y/Mutex.h>
#include <c9y/Condition.h>
#include <c9y/Timer.h>

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
        c9y::Mutex mutex;
        Network& network;
        NodePtr node;
        c9y::Condition connect_condition;
        c9y::Timer timer;
        unsigned int retry_count; 

        enum State
        {
            INIT,
            WAITING,
            CONNECTED,
            FAILED
        } state;       
                
        void on_message_recived(MessagePtr message);
        void on_timeout();
    };
}

#endif

// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _TEST_TRACER_H_
#define _TEST_TRACER_H_

#include "Network.h"

using namespace std;
using namespace std::tr1;
using namespace spdr;

class Tracker
{
public:
    
    Tracker(spdr::Network& network)        
    {
        network.node_connected.connect(sigc::mem_fun(this, &Tracker::track_node_connected));
        network.node_timeout.connect(sigc::mem_fun(this, &Tracker::track_node_timeout));
        network.node_disconnected.connect(sigc::mem_fun(this, &Tracker::track_node_disconnected));
        network.message_recived.connect(sigc::mem_fun(this, &Tracker::track_message_recived));
    }
    
    void track_node_connected(spdr::NodePtr node)
    {
        transcript << "Node Connected: " << node->get_state() << std::endl;
        connection_condition.signal();
    }
    
    void track_node_timeout(spdr::NodePtr node)
    {
        transcript << "Node Timeout: " << node->get_state() << std::endl;
        connection_condition.signal();
        message_condition.signal(); // signal this too, since this may be 
        // an error on the line...
    }
    
    void track_node_disconnected(spdr::NodePtr node)
    {
        transcript << "Node Disconnect: " << node->get_state() << std::endl;
        connection_condition.signal();
        message_condition.signal(); // signal this too, since this may be 
        // an error on the line...
    }
    
    void track_message_recived(shared_ptr<Message> msg)
    {
        transcript << "Message Recived: " << msg->get_type() << std::endl;
        last_message = msg;
        message_condition.signal();
    }
    
    void wait_connection()
    {
        connection_condition.wait();
    }
    
    void wait_message()
    {
        message_condition.wait();
    }
    
    std::string get_tanscript() const
    {
        return transcript.str();
    }
    
    shared_ptr<Message> get_last_message() const
    {
        return last_message;
    }
    
private:        
    c9y::Condition connection_condition;
    c9y::Condition message_condition;
    
    std::stringstream transcript;
    shared_ptr<Message> last_message;
};

#endif

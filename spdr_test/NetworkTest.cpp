// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <stdexcept>
#include <UnitTest++/UnitTest++.h>
#include <c9y/utility.h>
#include <sstream>

#include "Network.h"
#include "GenericMessage.h"

SUITE(Network)
{
    #define TEST_PROTOCOLL_ID 132465
    
//------------------------------------------------------------------------------
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
        
        void track_message_recived(spdr::MessagePtr msg)
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
        
        spdr::MessagePtr get_last_message() const
        {
            return last_message;
        }
        
    private:        
        c9y::Condition connection_condition;
        c9y::Condition message_condition;
        
        std::stringstream transcript;
        spdr::MessagePtr last_message;
    };
    
//------------------------------------------------------------------------------
    TEST(create_client_style_network)
    {
        spdr::Network net(TEST_PROTOCOLL_ID);
        CHECK_EQUAL(TEST_PROTOCOLL_ID, net.get_protocol_id());
    }  

//------------------------------------------------------------------------------
    TEST(create_server_style_network)
    {
        spdr::Network net(TEST_PROTOCOLL_ID, 1338);        
        CHECK_EQUAL(TEST_PROTOCOLL_ID, net.get_protocol_id());
        CHECK_EQUAL(1338, net.get_connect_port());
    }
    
//------------------------------------------------------------------------------
    TEST(connect)
    {
        spdr::Network server(TEST_PROTOCOLL_ID, 1340);
        Tracker server_tracker(server);
        
        spdr::Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        spdr::NodePtr server_node;
        server_node = client.connect(spdr::Address(127,0,0,1, 1340));
        
        CHECK_EQUAL(spdr::Node::CONNECTING, server_node->get_state());
        
        client_tracker.wait_connection();
        
        CHECK_EQUAL(spdr::Node::CONNECTED, server_node->get_state());
        
        std::string ref = "Node Connected: CONNECTED\n";
        CHECK_EQUAL(ref, client_tracker.get_tanscript());
        CHECK_EQUAL(ref, server_tracker.get_tanscript());
    }

//------------------------------------------------------------------------------
    TEST(send)
    {
        spdr::Network server(TEST_PROTOCOLL_ID, 1341);
        Tracker server_tracker(server);
        
        spdr::Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        spdr::NodePtr node = client.connect(spdr::Address(127,0,0,1, 1341));
        
        client_tracker.wait_connection();
        
        std::string value = "Test";
        std::vector<char> data(value.begin(), value.end());
        spdr::MessagePtr msg(new spdr::GenericMessage(node, 1300, data));
        client.send(msg);
        
        server_tracker.wait_message();
        
        spdr::MessagePtr last_message = server_tracker.get_last_message();
        
        CHECK(last_message);
        CHECK_EQUAL(1300, last_message->get_type());
        CHECK(data == last_message->get_payload());
    }
    
//------------------------------------------------------------------------------    
    TEST(connect_failed)
    {
        spdr::Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        spdr::NodePtr node = client.connect(spdr::Address(127,0,0,1, 4000));
        CHECK_EQUAL(spdr::Node::CONNECTING, node->get_state());
                
        client_tracker.wait_connection();
        
        std::string ref = "Node Timeout: TIMEOUT\n";
        CHECK_EQUAL(ref, client_tracker.get_tanscript());
        
    }

//------------------------------------------------------------------------------    
    /*TEST(connection_rejected_on_different_protocoll_id)
    {
        spdr::Network server(TEST_PROTOCOLL_ID, 1343);        
        spdr::Network client(TEST_PROTOCOLL_ID+1);
        Tracker client_tracker(client);
        
        client.connect(spdr::Address(127,0,0,1, 1343));            
        client_tracker.wait_connection();
        
        std::string ref = "Node Disconnect: DISCONNECTED\n";
        CHECK_EQUAL(ref, client_tracker.get_tanscript());
        
    }*/
}


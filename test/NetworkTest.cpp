// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <UnitTest++/UnitTest++.h>
#include <c9y/utility.h>

#include "Network.h"
#include "GenericMessage.h"

SUITE(Network)
{
//------------------------------------------------------------------------------
    TEST(create_client_style_network)
    {
        spdr::Network net;
        CHECK(net.accepts_connections() == false);
    }  

//------------------------------------------------------------------------------
    TEST(create_server_style_network)
    {
        spdr::Network net(1338);        
        CHECK(net.accepts_connections() == true);
        CHECK_EQUAL(1338, net.get_connect_port());
    }
    
//------------------------------------------------------------------------------  
    struct ConnectFixture
    {
        spdr::NodePtr connect_node;
        
        void on_connect(spdr::NodePtr node)
        {
            connect_node = node;
        }
    };
    
//------------------------------------------------------------------------------
    TEST_FIXTURE(ConnectFixture, connect)
    {
        spdr::Network server(1340);
        server.node_connected.connect(sigc::mem_fun(this, &ConnectFixture::on_connect));
        
        spdr::Network client;
        spdr::NodePtr server_node = client.connect(spdr::Address(127,0,0,1, 1340));
        
        CHECK(server_node);
        CHECK(connect_node);
    }
    
//------------------------------------------------------------------------------  
    struct SendFixture
    {
        spdr::MessagePtr last_message;
        
        void on_message(spdr::MessagePtr msg)
        {
            last_message = msg;
        }
    };
    
//------------------------------------------------------------------------------
    TEST_FIXTURE(SendFixture, send)
    {
        spdr::Network server(1341);
        server.message_recived.connect(sigc::mem_fun(this, &SendFixture::on_message));
        
        spdr::Network client;
        spdr::NodePtr node = client.connect(spdr::Address(127,0,0,1, 1341));
        
        spdr::MessagePtr msg(new spdr::GenericMessage(node, 1300, "Test"));
        client.send(msg);
        
        // We need to sleep a little, since send returns instantanious, 
        // but the packet must be encoded / routed / decoded.
        c9y::sleep(100);
        
        CHECK(last_message);
        CHECK_EQUAL(1300, last_message->get_type());
        CHECK_EQUAL("Test", last_message->get_payload());
    }
}

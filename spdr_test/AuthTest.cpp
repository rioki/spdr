// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include <sigc++/sigc++.h>

#include "Network.h"
#include "Tracker.h"

using namespace std;
using namespace std::tr1;
using namespace spdr;

SUITE(Auth)
{
    #define TEST_PROTOCOLL_ID 26548

//------------------------------------------------------------------------------
    bool authenticate(shared_ptr<Node> node, string user, string pass)
    {
        if ((user == "rioki") && (pass == "cerberus"))
        {
            return true;
        }
        return false;
    }

//------------------------------------------------------------------------------
    TEST(connect_authenticated)
    {
        Network server(TEST_PROTOCOLL_ID, 1340, sigc::ptr_fun(&authenticate));
        Tracker server_tracker(server);
        
        Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        shared_ptr<Node> server_node;
        server_node = client.connect(spdr::Address(127,0,0,1, 1340), "rioki", "cerberus");
        
        CHECK_EQUAL(Node::CONNECTING, server_node->get_state());
        
        client_tracker.wait_connection();
        
        CHECK_EQUAL(Node::CONNECTED, server_node->get_state());
    }    

//------------------------------------------------------------------------------
    TEST(connect_authenticated_failed)
    {
        Network server(TEST_PROTOCOLL_ID, 1340, sigc::ptr_fun(&authenticate));
        Tracker server_tracker(server);
        
        Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        shared_ptr<Node> server_node;
        server_node = client.connect(spdr::Address(127,0,0,1, 1340), "haxor", "cerberus");
        
        CHECK_EQUAL(Node::CONNECTING, server_node->get_state());
        
        client_tracker.wait_connection();
        
        CHECK_EQUAL(Node::DISCONNECTED, server_node->get_state());
    }

//------------------------------------------------------------------------------
    TEST(auth_node_user_name)
    {
        Network server(TEST_PROTOCOLL_ID, 1340, sigc::ptr_fun(&authenticate));
        Tracker server_tracker(server);
        
        Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        shared_ptr<Node> server_node;
        client.connect(spdr::Address(127,0,0,1, 1340), "rioki", "cerberus");
        client_tracker.wait_connection();
        
        CHECK_EQUAL("rioki", client.get_this_node()->get_user_name());
        
        vector<shared_ptr<Node> > nodes = server.get_nodes();
        CHECK_EQUAL(1, nodes.size());
        CHECK_EQUAL("rioki", nodes[0]->get_user_name());
    }      
}

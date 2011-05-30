// spdr - easy networking
// Copyright 2011 Sean Farrell

#include <stdexcept>
#include <UnitTest++/UnitTest++.h>
#include <c9y/utility.h>
#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>
#include <sstream>

#include "Network.h"
#include "Tracker.h"

using namespace std;
using namespace std::tr1;
using namespace spdr;
using namespace musli;

SUITE(Network)
{
    #define TEST_PROTOCOLL_ID 132465    
    
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
        Network server(TEST_PROTOCOLL_ID, 1340);
        Tracker server_tracker(server);
        
        Network client(TEST_PROTOCOLL_ID);
        Tracker client_tracker(client);
        
        shared_ptr<Node> server_node;
        server_node = client.connect(spdr::Address(127,0,0,1, 1340));
        
        CHECK_EQUAL(Node::CONNECTING, server_node->get_state());
        
        client_tracker.wait_connection();
        
        CHECK_EQUAL(Node::CONNECTED, server_node->get_state());
        
        std::string ref = "Node Connected: CONNECTED\n";
        CHECK_EQUAL(ref, client_tracker.get_tanscript());
        CHECK_EQUAL(ref, server_tracker.get_tanscript());
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
    TEST(connection_rejected_on_different_protocoll_id)
    {
        spdr::Network server(TEST_PROTOCOLL_ID, 1343);        
        spdr::Network client(TEST_PROTOCOLL_ID+1);
        Tracker client_tracker(client);
        
        shared_ptr<Node> node = client.connect(spdr::Address(127,0,0,1, 1343));            
        client_tracker.wait_connection();
        
        CHECK_EQUAL(Node::TIMEOUT, node->get_state());       
    }
//------------------------------------------------------------------------------
    class TestMessage : public Message
    {
    public:
        
        TestMessage()
        : severity(0) {}
        
        TestMessage(unsigned int s, const string& m)
        : severity(s), message(m) {}
        
        TestMessage* clone() const
        {
            return new TestMessage(*this);
        }
        
        unsigned int get_type() const
        {
            return 1337;
        }

        vector<char> encode() const
        {
            vector<char> payload;
            
            MemoryPacker packer(payload);
            packer << severity << message;
            
            return payload;
        }
    
        void decode(const vector<char>& payload)
        {
            MemoryUnpacker unpacker(payload);
            unpacker >> severity >> message;
        }
        
        unsigned int get_severity() const
        {
            return severity;
        }
        
        const std::string& get_message() const
        {
            return message;
        }
    
    private:        
        unsigned int severity;
        std::string message;        
    };
    
//------------------------------------------------------------------------------
    TEST(send)
    {
        Network server(TEST_PROTOCOLL_ID, 1342);
        server.register_message<TestMessage>();
        Tracker server_tracker(server);
        
        Network client(TEST_PROTOCOLL_ID);
        client.register_message<TestMessage>();
        Tracker client_tracker(client);
        
        shared_ptr<Node> node = client.connect(spdr::Address(127,0,0,1, 1342));        
        client_tracker.wait_connection();
        
        shared_ptr<Message> msg(new TestMessage(1, "Dump the warp core!"));
        client.send(node, msg);
        
        server_tracker.wait_message();
        shared_ptr<TestMessage> rmsg = dynamic_pointer_cast<TestMessage>(server_tracker.get_last_message());
        
        CHECK(rmsg);        
        if (rmsg)
        {
            CHECK_EQUAL(1, rmsg->get_severity());
            CHECK_EQUAL("Dump the warp core!", rmsg->get_message());
        }
    }
}


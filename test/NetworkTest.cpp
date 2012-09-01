// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include <sigc++/sigc++.h>
#include <c9y/Condition.h>
#include <c9y/utility.h>

#include "Network.h"
#include "debug.h"

// This define is used to dertmain the port range to test on.
// That range may actually be used by some other application, then you just 
// need to change it.
#define BASE_PORT 8000

SUITE(NetworkTest)
{        
    TEST(default_constructible)
    {
        spdr::Network network(1);
        CHECK_EQUAL(1, network.get_protocol_id());
    }

    struct ConnectTimeoutFixture
    {
        unsigned int node_disconnect_count;
        spdr::PeerInfo node_disconnect_info;
        c9y::Condition disconnect_cond;
        
        ConnectTimeoutFixture()
        : node_disconnect_count(0) {}
        
        void on_node_disconnect(spdr::PeerInfo info)
        {
            node_disconnect_count++;
            node_disconnect_info = info;
            disconnect_cond.signal();
        }
        
        void wait_disconnect()
        {
            disconnect_cond.wait();
        }
        
    };
    
    TEST_FIXTURE(ConnectTimeoutFixture, connect_timeout)
    {
        spdr::Network client(2);
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectTimeoutFixture::on_node_disconnect));
        
        client.connect(spdr::Address(127, 0, 0, 1, BASE_PORT + 0));
        wait_disconnect();
        
        CHECK_EQUAL(1, node_disconnect_count);
    }

    struct ConnectFixture
    {
        
        unsigned int server_connected_count;
        unsigned int client_connected_count;
        unsigned int server_disconnected_count;
        unsigned int client_disconnected_count;
        c9y::Condition client_connect_cond;
        
        ConnectFixture()
        {
            server_connected_count = 0;
            client_connected_count = 0;
            server_disconnected_count = 0;
            client_disconnected_count = 0;
        }
        
        void on_server_connected(spdr::PeerInfo info)
        {
            server_connected_count++;
        }
        
        void on_client_connected(spdr::PeerInfo info)
        {
            client_connected_count++;
            client_connect_cond.signal();
        }
        
        void on_client_disconnected(spdr::PeerInfo info)
        {
            client_disconnected_count++;
            client_connect_cond.signal();            
        }
        
        void on_server_disconnected(spdr::PeerInfo info)
        {
            server_disconnected_count++;
            client_connect_cond.signal();            
        }
        
        void wait_client_connect()
        {            
            if (client_connected_count == 0)
            {
                client_connect_cond.wait();
            }
        }        
    };

    TEST_FIXTURE(ConnectFixture, connect)
    {
        spdr::Network server(3, BASE_PORT + 1);
        server.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_server_connected));
        
        spdr::Network client(3);
        client.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_connected));
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_disconnected));
                
        client.connect(spdr::Address(127, 0, 0, 1, BASE_PORT + 1));
        
        wait_client_connect();
        
        CHECK_EQUAL(1, client_connected_count);        
        CHECK_EQUAL(1, server_connected_count);
        CHECK_EQUAL(0, client_disconnected_count);        
        CHECK_EQUAL(0, server_disconnected_count);
        
    }
    
    TEST_FIXTURE(ConnectTimeoutFixture, keep_alive)
    {
        spdr::Network server(4, BASE_PORT + 2);
        
        spdr::Network client(4);
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectTimeoutFixture::on_node_disconnect));
                
        client.connect(spdr::Address(127, 0, 0, 1, BASE_PORT + 2));
        
        c9y::sleep(3000);
        
        CHECK_EQUAL(0, node_disconnect_count);        
    }
        
    enum MessageId
    {
        TEST_MESSAGE_ID = 33
    };
    
    class TestMessage : public spdr::Message
    {
    public:
    
        TestMessage() {}
        
        TestMessage(unsigned int u, const std::string& t)
        : uid(u), text(t) {}
        
        unsigned int get_uid() const
        {
            return uid;
        }
        
        const std::string& get_text() const
        {
            return text;
        }
        
        unsigned int get_id() const
        {
            return TEST_MESSAGE_ID;
        }
        
        void pack(std::ostream& os) const
        {
            spdr::pack(os, uid);
            spdr::pack(os, text);
        }
        
        void unpack(std::istream& is)
        {
            spdr::unpack(is, uid);
            spdr::unpack(is, text);
        }
        
    private:
        unsigned int uid;
        std::string text;
    };
    
    struct SendMessageFixture
    {
        spdr::Network server;        
        
        unsigned int server_message_count;
        unsigned int client_message_count;
        
        unsigned int uid1;
        unsigned int uid2;
        
        std::string text1;
        std::string text2;
        
        c9y::Condition awnser_cond;
        
        SendMessageFixture()
        : server(5, BASE_PORT + 3)
        {
            server_message_count = 0;
            client_message_count = 0;
        }
        
        
        void handle_server_message(spdr::PeerInfo info, spdr::Message& message)
        {
            server_message_count++;
            if (message.get_id() == TEST_MESSAGE_ID)
            {
                TestMessage& test_message = dynamic_cast<TestMessage&>(message);
                uid1 = test_message.get_uid();
                text1 = test_message.get_text();
                
                server.send(info, new TestMessage(1, "Ok batman!"));
            }
        }
        
        void handle_client_message(spdr::PeerInfo info, spdr::Message& message)
        {
            client_message_count++;
            if (message.get_id() == TEST_MESSAGE_ID)
            {
                TestMessage& test_message = dynamic_cast<TestMessage&>(message);
                uid2 = test_message.get_uid();
                text2 = test_message.get_text();
                awnser_cond.signal();
            }
        }
        
        void handle_client_dissconnect(spdr::PeerInfo info)
        {
            // we just got let off the hock, abort the test.
            awnser_cond.signal();
        }
        
        void wait_awnser()
        {
            awnser_cond.wait();
        }
    };

    TEST_FIXTURE(SendMessageFixture, send_message)
    {        
        server.register_message<TestMessage>(TEST_MESSAGE_ID);
        server.get_message_signal().connect(sigc::mem_fun(this, &SendMessageFixture::handle_server_message));
        
        spdr::Network client(5);
        client.register_message<TestMessage>(TEST_MESSAGE_ID);
        client.get_message_signal().connect(sigc::mem_fun(this, &SendMessageFixture::handle_client_message));
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &SendMessageFixture::handle_client_dissconnect));
        
                
        spdr::PeerInfo info = client.connect(spdr::Address(127, 0, 0, 1, BASE_PORT + 3));
        client.send(info, new TestMessage(0, "To the batmobile!"));
                
        wait_awnser();
        
        CHECK_EQUAL(1, server_message_count);
        CHECK_EQUAL(1, client_message_count);
        
        CHECK_EQUAL(0, uid1);
        CHECK_EQUAL(1, uid2);
        
        CHECK_EQUAL("To the batmobile!", text1);
        CHECK_EQUAL("Ok batman!", text2);        
    }
    
    TEST_FIXTURE(ConnectFixture, disconnect)
    {
        spdr::Network server(4, BASE_PORT + 4);
        server.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_server_connected));
        server.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_server_disconnected));
        
        spdr::Network client(4);
        client.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_connected));        
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_disconnected));
                
        client.connect(spdr::Address(127, 0, 0, 1, BASE_PORT + 4));
        
        wait_client_connect();
        client.disconnect(spdr::Address(127, 0, 0, 1, BASE_PORT + 4));  
        c9y::sleep(4500);        
        
        CHECK_EQUAL(1, client_connected_count);        
        CHECK_EQUAL(1, server_connected_count);    
        CHECK_EQUAL(1, client_disconnected_count);
        CHECK_EQUAL(1, server_disconnected_count);
    }
}

// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include <sigc++/sigc++.h>
#include <c9y/Condition.h>
#include <c9y/utility.h>

#include "Network.h"

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
        
        client.connect(spdr::Address(127, 0, 0, 1, 1337));
        wait_disconnect();
        
        CHECK_EQUAL(1, node_disconnect_count);
    }

    struct ConnectFixture
    {
        
        unsigned int server_connected_count;
        unsigned int client_connected_count;
        c9y::Condition client_connect_cond;
        
        ConnectFixture()
        {
            server_connected_count = 0;
            client_connected_count = 0;
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
            // We need alos to handle the connection failure...
            client_connect_cond.signal();
        }
        
        void wait_client_connect()
        {            
            client_connect_cond.wait();
        }        
    };

    TEST_FIXTURE(ConnectFixture, connect)
    {
        spdr::Network server(3, 1340);
        server.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_server_connected));
        
        spdr::Network client(3);
        client.get_connect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_connected));
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectFixture::on_client_disconnected));
                
        client.connect(spdr::Address(127, 0, 0, 1, 1340));
        
        wait_client_connect();
        
        CHECK_EQUAL(1, client_connected_count);        
        CHECK_EQUAL(1, server_connected_count);
        
    }
    
    TEST_FIXTURE(ConnectTimeoutFixture, keep_alive)
    {
        spdr::Network server(4, 1339);
        
        spdr::Network client(4);
        client.get_disconnect_signal().connect(sigc::mem_fun(this, &ConnectTimeoutFixture::on_node_disconnect));
                
        client.connect(spdr::Address(127, 0, 0, 1, 1339));
        
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
        : server(5, 2338)
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
        
                
        spdr::PeerInfo info = client.connect(spdr::Address(127, 0, 0, 1, 2338));
        client.send(info, new TestMessage(0, "To the batmobile!"));
                
        wait_awnser();
        
        CHECK_EQUAL(1, server_message_count);
        CHECK_EQUAL(1, client_message_count);
        
        CHECK_EQUAL(0, uid1);
        CHECK_EQUAL(1, uid2);
        
        CHECK_EQUAL("To the batmobile!", text1);
        CHECK_EQUAL("Ok batman!", text2);        
    }    
}

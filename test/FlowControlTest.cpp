// spdr - easy networking
// Copyright 2011-2012 Sean Farrell

#include <UnitTest++/UnitTest++.h>

#include <stdexcept>
#include <sigc++/sigc++.h>
#include <c9y/Thread.h>
#include <c9y/Condition.h>
#include <c9y/utility.h>

#include "Network.h"
#include "UdpSocket.h"

// This define is used to dertmain the port range to test on.
// That range may actually be used by some other application, then you just 
// need to change it.
#define BASE_PORT 8500
#define PROTOCOL_ID 1234

SUITE(FlowControlTest)
{ 
    enum MessageId
    {
        TEST_MESSAGE_ID = 1
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
        
        bool is_reliable() const
        {
            return true;
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
    
    float randf()
    {
        return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }
    
    struct FlowControlFixture
    {
        float packet_loss;
        bool running;
        c9y::Thread thread;
        
        spdr::UdpSocket socket;
        unsigned short client_port;
        unsigned short server_port;
        
        std::vector<TestMessage> messages;
        c9y::Condition done_cond;
        
        
        FlowControlFixture()
        : packet_loss(0.3), running(true), thread(sigc::mem_fun(this, &FlowControlFixture::network_simulation)), 
          socket(BASE_PORT), client_port(0), server_port(0)
        {
            thread.start();
        }
        
        ~FlowControlFixture()
        {
            running = false;
            thread.join();
        }
        
        void network_simulation()
        {
            while (running)
            {
                spdr::Address adr;
                std::string buff;    
                std::tie(adr, buff) = socket.recive();
                
                if (! buff.empty())
                {
                    // the first packet is sent from the client and the client
                    // port is not known, so we need to save it.
                    if (client_port == 0)
                    {
                        client_port = adr.get_port();
                    }
                    
                    if (client_port == adr.get_port())
                    {
                        adr.set_port(server_port);
                    }
                    else if (server_port == adr.get_port())
                    {
                        adr.set_port(client_port);
                    }
                    else
                    {
                        throw std::logic_error("WTF?!");
                    }
                    
                    // TODO actual network sim
                    float r = randf();
                    
                    if (r > packet_loss)
                    {
                        socket.send(adr, buff);
                    }
                }
            }
        }
        
        void recive_message(spdr::PeerInfo peer, spdr::Message& message)
        {
            if (message.get_id() == TEST_MESSAGE_ID)
            {
                TestMessage& tm = dynamic_cast<TestMessage&>(message);
                messages.push_back(tm);
                
                if (messages.size() == 5)
                {
                    done_cond.signal();
                }
            }
        }
    };
    
    
    
    TEST_FIXTURE(FlowControlFixture, flow_control1)
    {
        server_port = BASE_PORT + 1;
        spdr::Network server(PROTOCOL_ID, server_port);
        server.register_message<TestMessage>(TEST_MESSAGE_ID);
        server.get_message_signal().connect(sigc::mem_fun(this, &FlowControlFixture::recive_message));
        
        spdr::Network client(PROTOCOL_ID);
        client.register_message<TestMessage>(TEST_MESSAGE_ID);        
        spdr::Address server_adr(127, 0, 0, 1, BASE_PORT);
        spdr::PeerInfo info = client.connect(server_adr);
                
        client.send(info, TestMessage(1, "One"));
        client.send(info, TestMessage(2, "Two"));
        client.send(info, TestMessage(3, "Three"));
        client.send(info, TestMessage(4, "Four"));
        client.send(info, TestMessage(5, "Five"));
        
        done_cond.wait();
                
        CHECK_EQUAL(5, messages.size());
        
    }
}
// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Network.h"

#include <cassert>
#include <stdexcept>
#include <algorithm>

#include <c9y/Lock.h>
#include <c9y/utility.h>
#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"
#include "GenericMessage.h"
#include "ConnectMessage.h"

namespace spdr
{    
//------------------------------------------------------------------------------
    Network::Network()
    : worker_thread(sigc::mem_fun(this, &Network::main))
    {
        init(0);
    }

//------------------------------------------------------------------------------    
    Network::Network(unsigned short cp)
    : worker_thread(sigc::mem_fun(this, &Network::main)),
      socket(cp)
    {
        init(cp);
    }    

//------------------------------------------------------------------------------
    Network::~Network() 
    {
        running = false;
        worker_thread.join();
    }

//------------------------------------------------------------------------------    
    unsigned short Network::get_connect_port() const
    {
        return connect_port;
    }

//------------------------------------------------------------------------------    
    bool Network::accepts_connections() const
    {
        return connect_port != 0;
    }

//------------------------------------------------------------------------------    
    NodePtr Network::connect(const Address& address)
    {
        NodePtr node(new Node(address));
        nodes.push_back(node);
        
        connect_condition.clear();
        
        // TODO: make protocoll version a define
        MessagePtr con_msg(new ConnectMessage(node, 1));
        send(con_msg);
                
        connect_condition.wait(); // TODO timeout
        //connect_condition.wait(500);
        
        return node;
    }

//------------------------------------------------------------------------------    
    void Network::send(MessagePtr message)
    {
        if (! message)
        {
            throw std::invalid_argument("message");
        }
        if (! message->to)
        {
            throw std::invalid_argument("message->to");
        }
        
        // We need to set the from field, since messages may be created with
        // a null from; which means that it is from this node.         
        if (! message->from)
        {
            message->from = get_this_node();
        }
        // limit scope for lock
        {
            c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
            send_queue.push(message);
        }
    }

//------------------------------------------------------------------------------
    NodePtr Network::get_this_node() const
    {
        return this_node;
    }
    
//------------------------------------------------------------------------------    
    void Network::init(unsigned short cp)
    {
        connect_port = cp;
        
        this_node = NodePtr(new Node);
        running = true;
        worker_thread.start();
    }
    
//------------------------------------------------------------------------------    
    void Network::main()
    {
        // locking 
        while (running)
        {
             // send 
            MessagePtr msg = get_next_message();
            if (msg)
            {
                send_message(msg);
            }
            
            // recive
            msg = recive_message();
            if (msg)
            {
                if (msg->get_type() < 128)
                {
                    handle_internal_message(msg);
                }
                else
                {
                    message_recived(msg);
                } 
            }
        }        
    }

//------------------------------------------------------------------------------
    MessagePtr Network::get_next_message()
    {
        c9y::Lock<c9y::Mutex> lock(send_queue_mutex);
        if (! send_queue.empty())
        {
            MessagePtr msg = send_queue.front();
            send_queue.pop();
            return msg;
        }
        return MessagePtr();
    }

//------------------------------------------------------------------------------    
    void Network::send_message(MessagePtr msg)
    {
        assert(msg);
        assert(msg->get_to());
        
        Address adr = msg->get_to()->get_address();
        
        std::vector<char> buff;
        musli::MemoryPacker packer(buff);
        packer << msg->get_type() << msg->get_payload();
                
        socket.send(adr, buff);
    }

//------------------------------------------------------------------------------        
    MessagePtr Network::recive_message()
    {
        Address address;
        std::vector<char> buff;
        std::tr1::tie(address, buff) = socket.recive();
        if (! buff.empty())
        {
            NodePtr to = get_this_node();
            NodePtr from = get_node_from_address(address);
            unsigned int type;
            std::vector<char> payload;
            
            musli::MemoryUnpacker unpacker(buff);
            unpacker >> type >> payload;
            
            return create_message(to, from, type, payload);
        }
        else
        {
            return MessagePtr();
        }
    }

//------------------------------------------------------------------------------    
    struct address_equals
    {
        const Address& address;
        
        address_equals(const Address& a)
        : address(a) {}
        
        bool operator () (NodePtr node)
        {
            return address == node->get_address();
        }
    };

//------------------------------------------------------------------------------
    NodePtr Network::get_node_from_address(const Address& address)
    {
        std::vector<NodePtr>::iterator iter;
        iter = std::find_if(nodes.begin(), nodes.end(), address_equals(address));
        if (iter != nodes.end())
        {
            return *iter;
        }
        else
        {
            return NodePtr(new Node(address));
        }
    }

//------------------------------------------------------------------------------    
    MessagePtr Network::create_message(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload)
    {
        switch (type)
        {
            case CONNECT:
                return MessagePtr(new ConnectMessage(to, from, payload)); 
            default:
                return MessagePtr(new GenericMessage(to, from, type, payload)); 
        }
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_internal_message(MessagePtr msg)
    {
        switch (msg->get_type())
        {
            case CONNECT:
            {
                handle_connect(msg);
                break;
            }
            case ACK_CONNECT:
            {
                handle_ack_connect(msg);
                break;
            }            
            default:
            {
                assert(false && "Unknown internal message.");
                break;
            }
        }
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connect(MessagePtr m)
    {
        std::tr1::shared_ptr<ConnectMessage> msg = std::tr1::dynamic_pointer_cast<ConnectMessage>(m);
        assert(msg);
        
        NodePtr from = msg->get_from();
        node_connected(from);
        nodes.push_back(from);
        
        MessagePtr ack_msg(new GenericMessage(from, ACK_CONNECT, std::vector<char>()));
        send(ack_msg);
    }

//------------------------------------------------------------------------------    
    void Network::handle_ack_connect(MessagePtr msg)
    {
        connect_condition.signal();
    }
}
    
    
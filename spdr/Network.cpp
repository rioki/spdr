// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Network.h"

#include <cassert>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <c9y/Lock.h>
#include <c9y/utility.h>
#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"
#include "GenericMessage.h"
#include "Connect.h"
#include "ConnectionAccepted.h"
#include "ConnectionRejected.h"

namespace spdr
{    
//------------------------------------------------------------------------------
    unsigned int get_time()
    {
        return std::clock() / (CLOCKS_PER_SEC / 1000);
    }    
    
//------------------------------------------------------------------------------
    Network::Network(unsigned int pi)
    : protocol_id(pi), worker_thread(sigc::mem_fun(this, &Network::main))
    {
        init(0);
    }

//------------------------------------------------------------------------------    
    Network::Network(unsigned int pi, unsigned short cp)
    : protocol_id(pi), worker_thread(sigc::mem_fun(this, &Network::main)),
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
    unsigned int Network::get_protocol_id() const
    {
        return protocol_id;
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
        NodePtr node = create_node(address);
        node->last_message_recived = get_time();
        
        MessagePtr conect_message(new Connect(node, protocol_id));
        send(conect_message);
        
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
        send_queue.push(message);        
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
    NodePtr Network::create_node(const Address& address)
    {
        NodePtr node(new Node(address));
        nodes.add(node);
        return node;
    }

//------------------------------------------------------------------------------    
    void Network::remove_node(NodePtr node)
    {
        nodes.remove(node);
    }

//------------------------------------------------------------------------------    
    NodePtr Network::get_node_from_address(const Address& address)
    {
        NodePtr node = nodes.get_node_by_address(address);
        if (node)
        {
            return node;
        }
        else
        {
            return NodePtr(new Node(address));
        }
    }
    
//------------------------------------------------------------------------------
    void Network::main()
    {
        while (running)
        {
            // send 
            MessagePtr msg = send_queue.pop();
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
            
            check_node_timeout();
        }                
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
    MessagePtr Network::create_message(NodePtr to, NodePtr from, unsigned int type, const std::vector<char>& payload)
    {
        switch (type)
        {
            case CONNECT:
                return MessagePtr(new Connect(to, from, payload)); 
            case CONNECTION_ACCEPTED:
                return MessagePtr(new ConnectionAccepted(to, from, payload)); 
            case CONNECTION_REJECTED:
                return MessagePtr(new ConnectionRejected(to, from, payload));             
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
            case CONNECTION_ACCEPTED:
            {
                handle_connection_accepted(msg);
                break;
            }                
            case CONNECTION_REJECTED:
            {
                handle_connection_rejected(msg);
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
        std::tr1::shared_ptr<Connect> msg = std::tr1::dynamic_pointer_cast<Connect>(m);
        assert(msg);
        
        if (protocol_id == msg->get_protocol_id())
        {        
            NodePtr from = msg->get_from();
            from->set_state(Node::CONNECTED);
            node_connected(from);
            nodes.add(from);
            
            MessagePtr ack_msg(new ConnectionAccepted(from));
            send(ack_msg);
        }
        else
        {
            NodePtr from = msg->get_from();            
            
            MessagePtr ack_msg(new ConnectionRejected(from));
            send(ack_msg);
        }
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connection_accepted(MessagePtr m)
    {
        std::tr1::shared_ptr<ConnectionAccepted> msg = std::tr1::dynamic_pointer_cast<ConnectionAccepted>(m);
        assert(msg);
        
        NodePtr from = msg->get_from();
        from->set_state(Node::CONNECTED);
        node_connected(from);
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connection_rejected(MessagePtr m)
    {
        // TODO: add reason to rejection
        
        std::tr1::shared_ptr<ConnectionRejected> msg = std::tr1::dynamic_pointer_cast<ConnectionRejected>(m);
        assert(msg);
        
        NodePtr from = msg->get_from();
        from->set_state(Node::DISCONNECTED);
        node_disconnected(from);
        remove_node(from);
    }

//------------------------------------------------------------------------------    
    void Network::check_node_timeout()
    {
        std::vector<NodePtr> t_nodes = nodes.get_timout_nodes();
        for (unsigned int i = 0; i < t_nodes.size(); i++)
        {
            t_nodes[i]->set_state(Node::TIMEOUT);
            node_timeout(t_nodes[i]);
            remove_node(t_nodes[i]);
        }
    }
}
    
    
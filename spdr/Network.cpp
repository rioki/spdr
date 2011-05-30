// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Network.h"

#include <cassert>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <sanity/trace.h>
#include <sanity/check.h>
#include <c9y/Lock.h>
#include <c9y/utility.h>
#include <musli/MemoryPacker.h>
#include <musli/MemoryUnpacker.h>

#include "InternalMessageType.h"
#include "ConnectMessage.h"
#include "AcceptMessage.h"
#include "RejectMessage.h"
#include "KeepAliveMessage.h"

using namespace std;
using namespace std::tr1;

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
        node->last_message_sent = get_time();
        
        std::tr1::shared_ptr<Message> conect_message(new ConnectMessage(protocol_id));
        send(node, conect_message);
        
        return node;
    }

//------------------------------------------------------------------------------    
    void Network::send(std::tr1::shared_ptr<Node> node, std::tr1::shared_ptr<Message> message)
    {
        CHECK_ARGUMENT(message);
        
        message->set_to(node);
        message->set_from(get_this_node());
        
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
        
        register_message<ConnectMessage>();
        register_message<AcceptMessage>();
        register_message<RejectMessage>();
        register_message<KeepAliveMessage>();
        
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
            try
            {
                // send 
                std::tr1::shared_ptr<Message> msg = send_queue.pop();
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
                
                handle_keep_alive();
                check_node_timeout();            
            }
            catch (const std::exception& ex)
            {
                TRACE_ERROR(ex.what());
            }
        }                
    }

//------------------------------------------------------------------------------    
    void Network::send_message(std::tr1::shared_ptr<Message> msg)
    {
        CHECK_ARGUMENT(msg);
        CHECK_ARGUMENT(msg->get_to());
                
        Address adr = msg->get_to()->get_address();
        
        std::vector<char> buff;
        musli::MemoryPacker packer(buff);
        packer << msg->get_type() << msg->encode();
                
        socket.send(adr, buff);
        
        msg->get_to()->last_message_sent = get_time();
    }

//------------------------------------------------------------------------------        
    std::tr1::shared_ptr<Message> Network::recive_message()
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
            
            from->last_message_recived = get_time();
            
            return create_message(to, from, type, payload);
        }
        else
        {
            return std::tr1::shared_ptr<Message>();
        }
    }

//------------------------------------------------------------------------------    
    shared_ptr<Message> Network::create_message(shared_ptr<Node> to, shared_ptr<Node> from, unsigned int type, const std::vector<char>& payload)
    {
        shared_ptr<Message> message = message_factory.create(type);
        ASSERT(message);
        
        message->set_to(to);
        message->set_from(from);
        message->decode(payload);
        
        return message;
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_internal_message(std::tr1::shared_ptr<Message> msg)
    {
        switch (msg->get_type())
        {
            case CONNECT_MESSAGE:
            {
                handle_connect(msg);
                break;
            }
            case ACCEPT_MESSAGE:
            {
                handle_connection_accepted(msg);
                break;
            }                
            case REJECT_MESSAGE:
            {
                handle_connection_rejected(msg);
                break;
            }
            case KEEP_ALIVE_MESSAGE:
            {
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
    void Network::handle_connect(shared_ptr<Message> m)
    {
        shared_ptr<ConnectMessage> msg = dynamic_pointer_cast<ConnectMessage>(m);
        ASSERT(msg);
        
        if (protocol_id == msg->get_protocol_id())
        {        
            shared_ptr<Node> from = msg->get_from();
            from->set_state(Node::CONNECTED);
            node_connected(from);
            nodes.add(from);
                       
            send(from, shared_ptr<Message>(new AcceptMessage));
        }
        else
        {
            shared_ptr<Node> from = msg->get_from();
            send(from, shared_ptr<Message>(new RejectMessage));
        }
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connection_accepted(shared_ptr<Message> m)
    {
        shared_ptr<AcceptMessage> msg = dynamic_pointer_cast<AcceptMessage>(m);
        ASSERT(msg);
        
        shared_ptr<Node> from = msg->get_from();
        from->set_state(Node::CONNECTED);
        node_connected(from);
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connection_rejected(std::tr1::shared_ptr<Message> m)
    {
        // TODO: add reason to rejection
        
        shared_ptr<RejectMessage> msg = dynamic_pointer_cast<RejectMessage>(m);
        ASSERT(msg);
        
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
    
//------------------------------------------------------------------------------
    bool needs_keep_alive(NodePtr node)
    {
        return (get_time() - node->get_last_message_sent()) > 250;
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_keep_alive()
    {
        std::vector<NodePtr> ka_nodes = nodes.get_nodes(needs_keep_alive);
        for (unsigned int i = 0; i < ka_nodes.size(); i++)
        {
            shared_ptr<Message> ka(new KeepAliveMessage);
            ka->set_to(ka_nodes[i]);
            ka->set_from(this_node);
            
            send_message(ka);
        }
    }
}
    
    
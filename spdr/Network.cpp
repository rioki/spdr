// spdr - easy networking
// Copyright 2011 Sean Farrell

#include "Network.h"

#include <cassert>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

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
using namespace c9y;

namespace spdr
{    
//------------------------------------------------------------------------------
    template <typename Type>
    std::string str(Type value)
    {
        std::stringstream buff;
        buff << value;
        return buff.str();
    }
    
//------------------------------------------------------------------------------
    unsigned int get_time()
    {
        return std::clock() / (CLOCKS_PER_SEC / 1000);
    }   

//------------------------------------------------------------------------------    
    bool false_auth_func(shared_ptr<Node> node, string user, string pass) 
    {
        return false;
    }
     

//------------------------------------------------------------------------------    
    bool true_auth_func(shared_ptr<Node> node, string user, string pass) 
    {
        return true;
    }
    
//------------------------------------------------------------------------------
    Network::Network(unsigned int pi)
    : protocol_id(pi), worker_thread(sigc::mem_fun(this, &Network::main)),
      auth_func(sigc::ptr_fun(false_auth_func))
    {
        init(0);
    }

//------------------------------------------------------------------------------    
    Network::Network(unsigned int pi, unsigned short cp)
    : protocol_id(pi), worker_thread(sigc::mem_fun(this, &Network::main)),
      socket(cp), auth_func(sigc::ptr_fun(true_auth_func))
    {
        init(cp);
    }

//------------------------------------------------------------------------------
    Network::Network(unsigned int pi, unsigned short cp, sigc::slot<bool, std::tr1::shared_ptr<Node>, std::string, std::string> af)    
    : protocol_id(pi), worker_thread(sigc::mem_fun(this, &Network::main)),
      socket(cp), auth_func(af)
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
    NodePtr Network::connect(const Address& address, const std::string& user, const std::string& pass)
    {
        TRACE_INFO("Connecting to " + str(address) + " with username " + user + ".");
        
        NodePtr node = create_node(address);
        node->last_message_recived = get_time();
        node->last_message_sent = get_time();
        
        std::tr1::shared_ptr<Message> conect_message(new ConnectMessage(user, pass));
        send(node, conect_message);
        
        this_node->user_name = user;
        
        return node;
    }

//------------------------------------------------------------------------------        
    std::vector<std::tr1::shared_ptr<Node> > Network::get_nodes() const
    {
        Lock<Mutex> lock(nodes_mutex);
        std::vector<std::tr1::shared_ptr<Node> > copy = nodes;
        
        return copy;
    }

//------------------------------------------------------------------------------    
    void Network::send(std::tr1::shared_ptr<Node> node, std::tr1::shared_ptr<Message> message)
    {
        CHECK_ARGUMENT(message);
        TRACE_DEBUG("Enquing message of type " + str(message->get_type()) + ".");
        
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
    shared_ptr<Node> Network::create_node(const Address& address)
    {
        shared_ptr<Node> node(new Node(address));
        add_node(node);
        return node;
    }

//------------------------------------------------------------------------------    
    void Network::add_node(shared_ptr<Node> node)
    {
        Lock<Mutex> lock(nodes_mutex);
        nodes.push_back(node);
    }

//------------------------------------------------------------------------------    
    void Network::remove_node(shared_ptr<Node> node)
    {
        Lock<Mutex> lock(nodes_mutex);
        
        vector<shared_ptr<Node> >::iterator iter;
        iter = find(nodes.begin(), nodes.end(), node);
        ASSERT(iter != nodes.end());
        nodes.erase(iter);
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
        Lock<Mutex> lock(nodes_mutex);
        
         vector<shared_ptr<Node> >::iterator iter;
        iter = std::find_if(nodes.begin(), nodes.end(), address_equals(address));
        
        if (iter != nodes.end())
        {
            return *iter;
        }
        else
        {
            return shared_ptr<Node>(new Node(address));
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
        TRACE_DEBUG("Sending message of type " + str(msg->get_type()) + " to " + str(msg->get_to()->get_address()) + ".");
                
        Address adr = msg->get_to()->get_address();
        
        std::vector<char> buff;
        musli::MemoryPacker packer(buff);
        packer << protocol_id << msg->get_type() << msg->encode();
                
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
            
            unsigned int pi;
            unsigned int type;
            std::vector<char> payload;
            
            musli::MemoryUnpacker unpacker(buff);
            unpacker >> pi >> type >> payload;
            
            if (pi == protocol_id)
            {            
                NodePtr to = get_this_node();
                NodePtr from = get_node_from_address(address);
                from->last_message_recived = get_time();
                
                TRACE_DEBUG("Recived message of type " + str(type) + " form " + str(from->get_address()) + ".");
                
                return create_message(to, from, type, payload);
            }
        }
        return shared_ptr<Message>();
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
        
        shared_ptr<Node> from = msg->get_from();
        string user = msg->get_user();
        string pass = msg->get_pass();
        
        if (auth_func(from, user, pass))
        {        
            TRACE_INFO("Connection initiated by node " + str(from->get_address()) + ".");
            from->set_state(Node::CONNECTED);
            from->user_name = user;            
            node_connected(from);
            add_node(from);
                        
                       
            send(from, shared_ptr<Message>(new AcceptMessage));
        }
        else
        {
            TRACE_INFO("Connection rejected to node " + str(from->get_address()) + ".");
            send(from, shared_ptr<Message>(new RejectMessage));
        }
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_connection_accepted(shared_ptr<Message> m)
    {
        shared_ptr<AcceptMessage> msg = dynamic_pointer_cast<AcceptMessage>(m);
        ASSERT(msg);
        
        shared_ptr<Node> from = msg->get_from();
        TRACE_INFO("Connection initiated to node " + str(from->get_address()) + ".");
        
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
        TRACE_INFO("Connection rejected by node " + str(from->get_address()) + ".");
        
        from->set_state(Node::DISCONNECTED);
        node_disconnected(from);
        remove_node(from);
    }

//------------------------------------------------------------------------------        
    vector<shared_ptr<Node> > get_timeout_nodes(const vector<shared_ptr<Node> >& nodes)
    {
        vector<shared_ptr<Node> > result;
        
        unsigned int now = get_time();
        
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            if ((now - nodes[i]->get_last_message_recived()) > 500)
            {
                result.push_back(nodes[i]);
            }
        }
        
        return result;
    }

//------------------------------------------------------------------------------    
    void Network::check_node_timeout()
    {
        Lock<Mutex> lock(nodes_mutex);
                
        std::vector<NodePtr> t_nodes = get_timeout_nodes(nodes);
        for (unsigned int i = 0; i < t_nodes.size(); i++)
        {
            TRACE_INFO("Connection timedout to node " + str(t_nodes[i]->get_address()) + ".");
            t_nodes[i]->set_state(Node::TIMEOUT);
            node_timeout(t_nodes[i]);
            remove_node(t_nodes[i]);
        }
    }
    
//------------------------------------------------------------------------------        
    vector<shared_ptr<Node> > get_keep_alive_nodes(const vector<shared_ptr<Node> >& nodes)
    {
        vector<shared_ptr<Node> > result;        
        unsigned int now = get_time();
        
        for (unsigned int i = 0; i < nodes.size(); i++)
        {
            if ((now - nodes[i]->get_last_message_sent()) > 250)
            {
                result.push_back(nodes[i]);
            }
        }
        
        return result;
    }
    
//------------------------------------------------------------------------------    
    void Network::handle_keep_alive()
    {
        Lock<Mutex> lock(nodes_mutex);
        
        vector<shared_ptr<Node> > ka_nodes = get_keep_alive_nodes(nodes);
        for (unsigned int i = 0; i < ka_nodes.size(); i++)
        {
            TRACE_DEBUG("Sending keep alive to node " + str(ka_nodes[i]->get_address()) + ".");
            shared_ptr<Message> ka(new KeepAliveMessage);
            ka->set_to(ka_nodes[i]);
            ka->set_from(this_node);
            
            send_message(ka);
        }
    }
}
    
    
// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _SPDR_MESSAGE_H_
#define _SPDR_MESSAGE_H_

#include <tr1/memory>
#include <vector>

#include "Node.h"

namespace spdr
{
    /**
     * Message
     **/
    class Message
    {
    public:
        /**
         * Constructor 
         **/
        Message();
        
        /**
         * Denstructor 
         **/
        virtual ~Message();
        
        /**
         * Make a copy of this message. 
         **/
        virtual Message* clone() const = 0;
        
        /**
         * Get the message type id. 
         **/
        virtual unsigned int get_type() const = 0;
        
        /**
         * Set the message's recipient. 
         **/
        void set_to(std::tr1::shared_ptr<Node> value);
        
        /**
         * Get the message's recipient. 
         **/
        std::tr1::shared_ptr<Node> get_to() const;
        
        /**
         * Set the message's sender. 
         **/
        void set_from(std::tr1::shared_ptr<Node> value);
        
        /**
         * Get the message's sender. 
         **/
        std::tr1::shared_ptr<Node> get_from() const;        

        /**
         * Encode the message to a memory buffer; 
         **/
        virtual std::vector<char> encode() const = 0;
    
        /**
         * Decode the message from a memory buffer; 
         **/
        virtual void decode(const std::vector<char>& paylod) = 0;

    private:        
        std::tr1::shared_ptr<Node> to;
        std::tr1::shared_ptr<Node> from;
    
    friend class Network;
    };    
}

#endif

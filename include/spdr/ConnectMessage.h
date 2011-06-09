// spdr - easy networking
// Copyright 2011 Sean Farrell

#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "Message.h"

#include <string>

namespace spdr
{
    /**
     * Message used to establish connections.
     **/
    class ConnectMessage : public Message
    {
    public:
        /**
         * Deault Constructor
         **/
        ConnectMessage();
        
        /**
         * Initialize ConnectionMessage
         **/
        ConnectMessage(const std::string& user, const std::string& pass);
        
        virtual ConnectMessage* clone() const;
        
        virtual unsigned int get_type() const;

        virtual std::vector<char> encode() const;
    
        virtual void decode(const std::vector<char>& paylod);

        const std::string& get_user() const;
        
        const std::string& get_pass() const;
        
    private:
        std::string user;
        std::string pass;
    };    
}

#endif

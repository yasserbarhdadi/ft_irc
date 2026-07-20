#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

class Message
{
    private: 
        std::string buffer;
    public:
        Message() {};
        Message(std::string &buf);
        ~Message() {};
        // void parse_Message(std::string &message);
};
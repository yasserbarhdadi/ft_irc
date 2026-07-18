#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

class Message
{
    private: 
        std::string command;
        std::string prefix;
        std::vector<std::string> parameters;
public:
    Message();
    ~Message();
    void parse_Message(std::string &message);
    std::string get_message();
    std::string get_command();
    std::string get_perfix();
    std::vector<std::string> get_parameters();

}
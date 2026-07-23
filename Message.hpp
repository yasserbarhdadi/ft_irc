#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

class Message
{
    private: 
        std::string cmd;
		std::vector<std::string> params;
    public:
        Message() {};
        ~Message() {};

		void parse(std::string &ln);
		std::string getCmd() const;
		std::vector<std::string> getParams() const;
};
#pragma once
#include "main.hpp"

class Client
{
    private:
        int socket;
        std::string ip_address;
        std::string nickname;
        std::string username;
        std::string realname;
        bool is_registered;
    public:
        Client();
        ~Client();
        Client(const Client &);
        Client(int sock, std::string ip_address, std::string nickname, std::string username, std::string realname, bool is_registered);
        Client &operator=(const Client &obj);
};
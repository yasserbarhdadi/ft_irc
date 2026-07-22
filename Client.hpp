#pragma once
#include "main.hpp"

class Client
{
    private:
        int fd;
        std::string ip_address;
        std::string nickname;
        std::string username;
        std::string realname;
        bool is_registered;
    public:
        Client();
        Client(int fd);
        Client(int fd, std::string ip_addr);
        ~Client();
        Client(const Client &);
        Client(int sock, std::string ip_address, std::string nickname, std::string username, std::string realname, bool is_registered);
        Client &operator=(const Client &obj);

        //added functions for pass permission
        bool	get_is_registered() const;
        void	set_is_registered(bool value);
};
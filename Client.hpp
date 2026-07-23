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
		std::string recv_buffer;
    public:
        Client();
        Client(int fd);
        Client(int fd, std::string ip_addr);
        ~Client();
        Client(const Client &);
        Client(int sock, std::string ip_address, std::string nickname, std::string username, std::string realname, bool is_registered);
        Client &operator=(const Client &obj);

        //added functions for pass permission
        bool		get_is_registered() const;
        void		set_is_registered(bool value);
		void 		push_back_buf(char *buf);
		std::string	&get_recv_buf();
};
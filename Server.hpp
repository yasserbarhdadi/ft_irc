#pragma once

#include "main.hpp"

class Server {
    private:
        std::string password;
        int port;
        int srv_socket;
        std::vector<struct pollfd> _pollfds;
        std::map<int, Client> client;
        void	send_reply(int fd, const std::string &reply);
		void	cmd_pass(int fd, Message &msg);
		void	cmd_nick(int fd, Message &msg);
		void	cmd_user(int fd, Message &msg);
    public:
        Server();
        ~Server();
        Server(const Server &);
        Server(std::string passwd, int prt);
        Server &operator=(const Server &);
    
        void run();
        void add_new_client();
        void parse_client_message(size_t &index);
		void dispatch_cmd(int fd, Message &msg);
};
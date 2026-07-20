#pragma once

#include "main.hpp"

class Server {
    private:
        std::string password;
        int port;
        int srv_socket;
        std::vector<struct pollfd> _pollfds;
        std::map<int, Client> client;
    public:
        Server();
        ~Server();
        Server(const Server &);
        Server(std::string passwd, int prt);
        Server &operator=(const Server &);
    
        void run();
};
#pragma once

#include "main.hpp"

class Server {
    private:
        std::string password;
        int port;
        int srv_socket;
    public:
        Server();
        ~Server();
        Server(const Server &);
        Server(std::string passwd, int prt);
        Server &operator=(const Server &);
    
        void run();
};
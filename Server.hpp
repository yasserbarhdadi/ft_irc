#pragma once

#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
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
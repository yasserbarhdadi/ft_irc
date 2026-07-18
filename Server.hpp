#pragma once

#include <string>
#include <sys/socket.h>
#include <sys/types.h>

class Server {
    private:
        std::string password;
        int port;
        int srv_socket;
    public:
        Server();
        ~Server();
        Server(const Server &);
        Server &operator=(const Server &);
        
        Server(std::string passwd, int prt);
        void run();
};
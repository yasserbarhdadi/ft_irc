#include "Server.hpp"

Server::Server()
{

}

Server::Server(const Server &other) : password(other.password), port(other.port)
{

}
Server::Server(std::string passwd, int prt) : password(passwd), port(prt)
{

}

Server::~Server()
{

}

Server& Server::operator=(const Server &obj)
{
    if(this != &obj)
    {
        this->password = obj.password;
        this->port = obj.port;
        this->srv_socket = obj.srv_socket;
    }
    return *this;
}

void Server::run()
{
    int srv_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv;

    memset(&serv,0,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8080);
    inet_pton(AF_INET,"127.0.0.0", &serv.sin_addr);
    if(bind(srv_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0)
    {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }
    if(listen(srv_socket, 5) < 0)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return;
    }
    while (true)
    {
        // client connection handling code would go here
    }
}
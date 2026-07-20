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
    struct sockaddr_in serv;

    std::cout << "Starting server...\n";

    srv_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (srv_socket == -1) {
        std::cerr << "Error: could not create socket" << std::endl;
        return;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    if(bind(srv_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }

    if(listen(srv_socket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return;
    }

    std::cout << "IRC Server is online.\n";
    std::cout << "Listening on port " << port << std::endl;

    struct pollfd pfd;
    pfd.fd = srv_socket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    while (true)
    {
        if (poll(&_pollfds[0], _pollfds.size(), -1) < 0) {
            // handle error
        }
        else {
            for (size_t i = 0; i < _pollfds.size(); i++)
            {
                if (_pollfds[i].revents & POLLIN)
                {
                    if (_pollfds[i].fd == srv_socket) {
                        // We have a new connection!
                        // TODO: Call accept() here
                        // TODO: Create a new pollfd for them and push_back to _pollfds
                    }
                    else {
                        // We have a message from someone already inside!
                        // TODO: Call recv() using _pollfds[i].fd
                        // TODO: Handle the text they sent
                    }
                }
            }
        }
    }
}
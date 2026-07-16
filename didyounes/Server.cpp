/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 16:06:03 by yel-joul          #+#    #+#             */
/*   Updated: 2026/06/13 16:06:04 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int f_port, std::string f_password) : _server_fd(-1) , _port(f_port) , _password(f_password)
{

}
Server::~Server()
{
    if (_server_fd != -1)
        close(_server_fd);

}

void Server::acceptNewClient()
{
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    int client_fd = accept(_server_fd, (struct sockaddr *)&cli_addr, &len);
    if (client_fd == -1) 
        return;
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    struct pollfd cli_pollfd;
    cli_pollfd.fd = client_fd;       
    cli_pollfd.events = POLLIN;      
    cli_pollfd.revents = 0;         
    _fds.push_back(cli_pollfd);
    std::cout << "New client connected! FD: " << client_fd << std::endl;
}

void Server::init()
{
    struct sockaddr_in ser_addr;
    int opt = 1;
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1)
        throw std::runtime_error("Failed to create socket");
    if ((setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) == -1)
        throw std::runtime_error("Address already in use");
    if ((fcntl(_server_fd, F_SETFL, O_NONBLOCK)) == -1 )
        throw std::runtime_error("something blocked");
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = INADDR_ANY;
    ser_addr.sin_port = htons(_port);
    if ((bind(_server_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr))) == -1)
        throw std::runtime_error("failed to bind");
    if ((listen(_server_fd, SOMAXCONN)) == -1)
        throw std::runtime_error("failed to listen");
    std::cout << "Server is perfectly listening on port " << _port << std::endl;
}

void Server::run()
{
    int bytes;
    char buffer[1024];
    struct pollfd server_pollfd;
    server_pollfd.fd = _server_fd;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    _fds.push_back(server_pollfd);
    while (true)
    {
        if (poll(&_fds[0], _fds.size(), -1) == -1)
            throw std::runtime_error("poll() failed");
        if (_fds[0].revents & POLLIN)
            acceptNewClient();
        for (size_t i = 1; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                memset(buffer, 0, sizeof(buffer));
                
                bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0); 

                if (bytes <= 0) 
                {
                    std::cout << "Client " << _fds[i].fd << " disconnected." << std::endl;
                    close(_fds[i].fd); 
                    _fds.erase(_fds.begin() + i); 
                    i--;
                }
                else 
                    std::cout << "Client " << _fds[i].fd << " says: " << buffer;
            }
        }
    }
}
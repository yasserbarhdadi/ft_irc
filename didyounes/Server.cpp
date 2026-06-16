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

Server::Server(int f_port, std::string f_password) : _port(f_port) , _password(f_password) , _server_fd(-1)
{

}
Server::~Server()
{
    if (_server_fd != -1)
        close(_server_fd);

}
void Server::init()
{
    int opt = 1;
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1)
        throw std::runtime_error("Failed to create socket");
    if ((setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) == -1)
        throw std::runtime_error("Address already in use");
    if ((fcntl(_server_fd, F_SETFL, O_NONBLOCK)) == -1 )
        throw std::runtime_error("something blocked");
}
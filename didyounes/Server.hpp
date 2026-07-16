/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 16:05:48 by yel-joul          #+#    #+#             */
/*   Updated: 2026/06/13 16:05:49 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP 
#define SERVER_HPP

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <poll.h>
#include <vector>
#include <cstring>


class Server
{
    private:
            int _server_fd;
            int _port;
            std::vector<struct pollfd>  _fds;
            std::string _password;
    public:
            Server(int f_port, std::string f_password);
            void        init();
            void        run();
            void        acceptNewClient();
            ~Server();
};
#endif
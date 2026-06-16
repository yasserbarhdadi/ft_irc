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

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>


class Server
{
    private:
            int _server_fd;
            int _port;
            std::string _password;
    public:
            Server(int f_port, std::string f_password);
            void    init();
            ~Server();
};
#endif
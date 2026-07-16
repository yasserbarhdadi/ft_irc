/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:30:30 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/21 14:30:39 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include "Client.hpp"
#include "cmd_handler.hpp"
#include "Server.hpp"
#include <exception>
#include <cstdlib>

Message parse_message(const std::string &raw_text)
{
	Message msg;
	std::istringstream ss(raw_text);
	std::string token;
	if (ss >> token)
		msg.set_cmd(token);
	while (ss >> token)
		msg.add_av(token);
	return msg;
}


int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cout << "Usage: ./irc_server <port> <password>" << std::endl;
        return 1;
    }

    try
    {
        int port = std::atoi(av[1]);
        Server irc(port, av[2]);
        
        irc.init();
        irc.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
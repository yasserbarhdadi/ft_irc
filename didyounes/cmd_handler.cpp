/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_handler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:57:00 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/21 15:24:39 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cmd_handler"

void handle_pass(Client &client, const Message &msg)
{
	std::vectorstd::string args;
	
}

void handle_nick(Client &client, const Message &msg)
{
	std::cout << "nick Command triaggered" << std::endl;
}

void handle_user(Client &client, const Message &msg)
{
	std::cout << "user Command triaggered" << std::endl;
}

void execute_cmd(Client &client, const Message &msg)
{
	std::string cmd;
	cmd = msg.get_cmd();
	if (cmd == "PASS")
		handle_pass(client, msg);
	else if (cmd == "NICK")
		handle_nick(client, msg);
	else if (cmd == "USER")
		handle_user(client, msg);
	else
		std::cout << "Unknown command" << std::endl;	
}

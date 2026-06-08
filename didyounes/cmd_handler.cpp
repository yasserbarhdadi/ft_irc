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
	std::vector<std::string> args;
	args = msg.get_av();
	if (args.size() == 0)
	{
		std::cout << "Error: Not enough parameters" << std::endl;
		return ;
	}
	if (args[0] == "12345") // This value was created for testing purposes only , i need to make sure to replace here with the correct one 
		std::cout << "Pass" << std::endl;
	else
		std::cout << "fail" << std::endl;
	
}

void handle_nick(Client &client, const Message &msg)
{
	std::vector<std::string> args;
	args = msg.get_av();
	if (args.size() == 0)
	{
		std::cout << "Error: No nickname given" << std::endl;
		return ;
	}
	client.set_nickname(args[0]);
	std::cout << "Client FD [" << client.get_fd() << "] set nickname to " << args[0] << std::endl;
}

void handle_user(Client &client, const Message &msg)
{
	std::vector<std::string> args;
	args = msg.get_av();
	if (args.size() < 4)
	{
		std::cout << "Error: Not enough parameters" << std::endl;
		return ;
	}
	client.set_username(args[0]);
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

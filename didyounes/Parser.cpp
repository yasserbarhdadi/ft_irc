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

int main()
{
	std::string test_input = "PRIVMSG didyounes hola";
	Message my_msg = parse_message(test_input);
	std::cout << "Command: " << my_msg.get_cmd() << std::endl;
	if (my_msg.get_av().size() >= 2)
	{
		std::cout << "Param 1: " << my_msg.get_av()[0] << std::endl;
		std::cout << "Param 2: " << my_msg.get_av()[1] << std::endl;
	}
	return 0;
}

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

// int main()
// {
// 	std::string test_input = "PRIVMSG didyounes hola";
// 	Message my_msg = parse_message(test_input);
// 	std::cout << "Command: " << my_msg.get_cmd() << std::endl;
// 	if (my_msg.get_av().size() >= 2)
// 	{
// 		std::cout << "Param 1: " << my_msg.get_av()[0] << std::endl;
// 		std::cout << "Param 2: " << my_msg.get_av()[1] << std::endl;
// 	}
// 	return 0;
// }


int main()
{
    std::cout << "=== Starting ft_irc Router & Security Tests ===" << std::endl;

    Client test_client(4);

    std::cout << "\n[Test 1] Valid PASS command:" << std::endl;
    Message msg1 = parse_message("PASS 12345"); 
    execute_cmd(test_client, msg1);
    std::cout << "\n[Test 2] Invalid PASS command:" << std::endl;
    Message msg2 = parse_message("PASS wrong_pass");
    execute_cmd(test_client, msg2);

    std::cout << "\n[Test 3] Valid NICK command:" << std::endl;
    Message msg3 = parse_message("NICK mr_boot_ola_00");
    execute_cmd(test_client, msg3);

    std::cout << "\n[Test 4] Security Guard Test (Empty NICK):" << std::endl;
    Message msg4 = parse_message("NICK");
    execute_cmd(test_client, msg4);
    std::cout << "\n[Test 5] Unknown Command Test:" << std::endl;
    Message msg5 = parse_message("HACK server");
    execute_cmd(test_client, msg5);

    std::cout << "\n=== Tests Completed Successfully ===" << std::endl;
    
    return 0;
}
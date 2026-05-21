/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:25:43 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 16:42:34 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Message()
{

}

Message::~Message()
{

}

std::string Message::get_cmd()
{
	return cmd;
}

std::vector<std::string> Message::get_av()
{
	return av;
}

void Message::set_cmd(const std::string &new_cmd)
{
	cmd = new_cmd;
}

void Message::add_av(const std::string &param)
{
	av.push_back(param);
}

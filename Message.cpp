/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:25:43 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 21:23:56 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Message()
{
}

Message::~Message()
{
}

std::string Message::get_cmd() const
{
	return cmd;
}

std::vector<std::string> Message::get_av() const
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

void Message::clear()
{
	cmd.clear();
	av.clear();
}
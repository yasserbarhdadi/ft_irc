/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:09:21 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 21:23:55 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int client_fd) : fd(client_fd), is_registered(false)
{
}

Client::~Client()
{
}

int Client::get_fd() const
{
	return fd;
}

std::string Client::get_buffer() const
{
	return buffer;
}

bool Client::get_registered() const
{
	return is_registered;
}

void Client::set_registered(bool status)
{
	is_registered = status;
}

void Client::append_to_buffer(const std::string &text)
{
	buffer += text;
}

void Client::remove_from_buffer(std::string::size_type count)
{
	buffer.erase(0, count);
}
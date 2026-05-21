/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:09:21 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/19 15:57:50 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int client_fd): fd(client_fd) , is_registered(false)
{

}

int Client::get_fd()
{
	return fd;
}

std::string Client::get_buffer()
{
	return buffer;
}

bool Client::get_registered()
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

Client::~Client()
{

}

std::string Client::get_nickname()
{
	return nickname;
}

std::string Client::get_username()
{
	return username;
}

void Client::set_nickname(const std::string &nick)
{
	nickname = nick;
}

void Client::set_username(const std::string &user)
{
	username = user;
}

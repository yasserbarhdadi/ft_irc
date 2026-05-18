/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:09:21 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 16:22:37 by yel-joul         ###   ########.fr       */
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

void Client::append_to_buffer(std::string text)
{
	buffer += text;
}

Client::~Client()
{
}

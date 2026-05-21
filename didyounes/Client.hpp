/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 15:51:41 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/19 15:58:18 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>

class Client
{
	private:
		int fd;
		std::string buffer;
		std::string nickname;
		std::string username;
		bool is_registered;
	public:
		Client(int client_fd);
		~Client();
		int get_fd();
		std::string get_buffer();
		bool get_registered();
		std::string get_nickname();
		std::string get_username();
		void set_registered(bool status);
		void set_nickname(const std::string &nick);
		void set_username(const std::string &user);
		void	append_to_buffer(const std::string &text);
};


#endif

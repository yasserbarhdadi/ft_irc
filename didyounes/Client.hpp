/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 15:51:41 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 16:08:33 by yel-joul         ###   ########.fr       */
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
		bool is_registered;
	public:
		Client(int client_fd);
		~Client();
		int get_fd();
		std::string get_buffer();
		bool get_registered();
		void set_registered(bool status);
		void	append_to_buffer(std::string text);
};


#endif

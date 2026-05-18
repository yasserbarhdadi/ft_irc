/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 15:51:41 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 21:23:54 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int fd;
		std::string buffer;
		bool is_registered;

	public:
		explicit Client(int client_fd);
		~Client();
		int get_fd() const;
		std::string get_buffer() const;
		bool get_registered() const;
		void set_registered(bool status);
		void append_to_buffer(const std::string &text);
		void remove_from_buffer(std::string::size_type count);
};

#endif
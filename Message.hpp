/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:25:37 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 21:23:56 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message
{
	private:
		std::string cmd;
		std::vector<std::string> av;

	public:
		Message();
		~Message();
		std::string get_cmd() const;
		std::vector<std::string> get_av() const;
		void set_cmd(const std::string &new_cmd);
		void add_av(const std::string &param);
		void clear();
};

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:25:37 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/19 16:05:35 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <iostream>
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
};


#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/18 16:25:37 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/18 16:38:19 by yel-joul         ###   ########.fr       */
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
		std::string get_cmd();
		std::vector<std::string> get_av();
		void set_cmd(std::string new_cmd);
		void add_av(std::string param);
};

#endif

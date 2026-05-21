/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_handler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yel-joul <yel-joul@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 14:56:43 by yel-joul          #+#    #+#             */
/*   Updated: 2026/05/21 15:05:48 by yel-joul         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CMD_HANDLER_HPP
#define CMD_HANDLER_HPP

#include "Message.hpp"
#include "Client.hpp"

void execute_cmd(Client &client, const Message &msg);
void handle_pass(Client &client, const Message &msg);
void handle_nick(Client &client, const Message &msg);
void handle_user(Client &client, const Message &msg);

#endif

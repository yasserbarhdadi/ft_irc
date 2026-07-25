#include "Server.hpp"

Server::Server()
	: _port(0), _srv_socket(-1)
{
}

Server::Server(const Server &o)
	: _password(o._password), _port(o._port), _srv_socket(-1), _hostname(o._hostname)
{
}

Server::Server(std::string passwd, int prt)
	: _password(passwd), _port(prt), _srv_socket(-1), _hostname(SERVER_NAME)
{
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		close(it->first);
	}

	if (this->_srv_socket != -1)
	{
		close(this->_srv_socket);
	}
}

Server &Server::operator=(const Server &o)
{
	if (this != &o)
	{
		this->_password = o._password;
		this->_port = o._port;
		this->_hostname = o._hostname;
	}
	return (*this);
}

/* ------------------------------------------------------------------------ */
/* Output helpers                                                           */
/* ------------------------------------------------------------------------ */

void Server::send_reply(int fd, const std::string &reply)
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);

	if (it == this->_clients.end())
	{
		return ;
	}

	it->second.queue_msg(reply + "\r\n");
}

void Server::flush_client(int fd)
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);

	if (it == this->_clients.end())
	{
		return ;
	}

	const std::string &data = it->second.get_send_buf();

	if (data.empty())
	{
		return ;
	}

	ssize_t sent = send(fd, data.c_str(), data.size(), 0);

	if (sent < 0)
	{
		return ;
	}

	it->second.consume_send_buf(static_cast<size_t>(sent));
}

void Server::sync_poll_events()
{
	for (size_t i = 0; i < this->_pollfds.size(); i++)
	{
		if (this->_pollfds[i].fd == this->_srv_socket)
		{
			continue ;
		}

		std::map<int, Client>::iterator it = this->_clients.find(this->_pollfds[i].fd);

		if (it == this->_clients.end())
		{
			continue ;
		}

		if (it->second.has_pending_output())
		{
			this->_pollfds[i].events = POLLIN | POLLOUT;
		}
		else
		{
			this->_pollfds[i].events = POLLIN;
		}
	}
}

std::string Server::rpl(const std::string &code, int fd)
{
	return (":" + this->_hostname + " " + code + " " + this->_clients[fd].get_nickname());
}

Client *Server::find_client_by_nick(const std::string &nick)
{
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if (it->second.get_nickname() == nick)
		{
			return (&it->second);
		}
	}
	return (NULL);
}

std::string Server::build_names_list(Channel &chan)
{
	std::string names;
	std::vector<Client*> members = chan.getMembers();

	for (size_t i = 0; i < members.size(); i++)
	{
		if (i > 0)
		{
			names += " ";
		}

		if (chan.isOperator(members[i]))
		{
			names += "@";
		}

		names += members[i]->get_nickname();
	}
	return (names);
}

void Server::remove_client(int fd, const std::string &reason)
{
	std::string prefix = ":" + this->_clients[fd].getPrefix();

	for (std::map<std::string, Channel>::iterator it = this->_channels.begin(); it != this->_channels.end();)
	{
		if (it->second.isMember(&this->_clients[fd]))
		{
			it->second.broadcast(prefix + " QUIT :" + reason, &this->_clients[fd]);
			it->second.removeClient(&this->_clients[fd]);
		}

		if (it->second.getMemberCount() == 0)
		{
			this->_channels.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

/* ------------------------------------------------------------------------ */
/* Registration: PASS / NICK / USER                                        */
/* ------------------------------------------------------------------------ */

void Server::cmd_pass(int fd, Message &msg)
{
	if (this->_clients[fd].get_is_registered())
	{
		this->send_reply(fd, ":" + this->_hostname + " 462 * :You may not reregister");
		return ;
	}

	if (msg.getParams().empty())
	{
		this->send_reply(fd, ":" + this->_hostname + " 461 * PASS :Not enough parameters");
		return ;
	}

	if (msg.getParams()[0] == this->_password)
	{
		this->_clients[fd].set_is_pass(true);
	}
	else
	{
		this->send_reply(fd, ":" + this->_hostname + " 464 * :Password incorrect");
	}
}

static bool is_valid_nick(const std::string &nick)
{
	if (nick.empty() || nick.size() > 9)
	{
		return (false);
	}

	if (!std::isalpha(static_cast<unsigned char>(nick[0])))
	{
		return (false);
	}

	for (size_t i = 1; i < nick.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(nick[i]);

		if (!std::isalnum(c) && c != '-' && c != '[' && c != ']'
			&& c != '\\' && c != '`' && c != '^' && c != '{'
			&& c != '|' && c != '}' && c != '_')
		{
			return (false);
		}
	}
	return (true);
}

void Server::cmd_nick(int fd, Message &msg)
{
	if (!this->_clients[fd].get_is_pass())
	{
		this->send_reply(fd, ":" + this->_hostname + " 451 * :You have not registered (Send PASS first)");
		return ;
	}

	if (msg.getParams().empty())
	{
		this->send_reply(fd, ":" + this->_hostname + " 431 * :No nickname given");
		return ;
	}

	std::string new_nick = msg.getParams()[0];

	if (!is_valid_nick(new_nick))
	{
		this->send_reply(fd, ":" + this->_hostname + " 432 * " + new_nick + " :Erroneous nickname");
		return ;
	}

	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if (it->second.get_nickname() == new_nick && it->first != fd)
		{
			this->send_reply(fd, ":" + this->_hostname + " 433 * " + new_nick + " :Nickname is already in use");
			return ;
		}
	}

	if (this->_clients[fd].get_is_registered())
	{
		std::string old_prefix = ":" + this->_clients[fd].getPrefix();

		this->_clients[fd].set_nickname(new_nick);
		for (std::map<std::string, Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
		{
			if (it->second.isMember(&this->_clients[fd]))
			{
				it->second.broadcast(old_prefix + " NICK " + new_nick, NULL);
			}
		}
		return ;
	}

	this->_clients[fd].set_nickname(new_nick);
	this->_clients[fd].set_is_nick(true);
	if (this->_clients[fd].try_register())
	{
		this->send_reply(fd, this->rpl("001", fd) + " :Welcome to the Internet Relay Network " + new_nick);
	}
}

void Server::cmd_user(int fd, Message &msg)
{
	if (!this->_clients[fd].get_is_pass())
	{
		this->send_reply(fd, ":" + this->_hostname + " 451 * :You have not registered (Send PASS first)");
		return ;
	}

	if (this->_clients[fd].get_is_registered())
	{
		this->send_reply(fd, ":" + this->_hostname + " 462 * :You may not reregister");
		return ;
	}

	if (msg.getParams().size() < 4)
	{
		this->send_reply(fd, ":" + this->_hostname + " 461 * USER :Not enough parameters");
		return ;
	}

	this->_clients[fd].set_username(msg.getParams()[0]);
	this->_clients[fd].set_realname(msg.getParams()[3]);
	this->_clients[fd].set_is_user(true);
	if (this->_clients[fd].try_register())
	{
		this->send_reply(fd, this->rpl("001", fd) + " :Welcome to the Internet Relay Network " + this->_clients[fd].get_nickname());
	}
}

/* ------------------------------------------------------------------------ */
/* JOIN / PART                                                              */
/* ------------------------------------------------------------------------ */

void Server::cmd_join(int fd, Message &msg)
{
	std::string nick = this->_clients[fd].get_nickname();

	if (msg.getParams().empty())
	{
		this->send_reply(fd, this->rpl("461", fd) + " JOIN :Not enough parameters");
		return ;
	}

	std::string chan_name = msg.getParams()[0];

	if (chan_name.empty() || chan_name[0] != '#')
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	bool is_new = (this->_channels.find(chan_name) == this->_channels.end());

	if (is_new)
	{
		this->_channels[chan_name] = Channel(chan_name);
	}

	Channel &chan = this->_channels[chan_name];

	if (chan.isMember(&this->_clients[fd]))
	{
		return ;
	}

	if (chan.isInviteOnly() && !chan.isInvited(nick))
	{
		this->send_reply(fd, this->rpl("473", fd) + " " + chan_name + " :Cannot join channel (+i)");
		return ;
	}

	if (!chan.getKey().empty())
	{
		std::string given;

		if (msg.getParams().size() > 1)
		{
			given = msg.getParams()[1];
		}

		if (given != chan.getKey())
		{
			this->send_reply(fd, this->rpl("475", fd) + " " + chan_name + " :Cannot join channel (+k)");
			return ;
		}
	}

	if (chan.getUserLimit() > 0 && chan.getMemberCount() >= chan.getUserLimit())
	{
		this->send_reply(fd, this->rpl("471", fd) + " " + chan_name + " :Cannot join channel (+l)");
		return ;
	}

	chan.addClient(&this->_clients[fd]);
	chan.removeInvite(nick);
	if (is_new)
	{
		chan.giveOperator(&this->_clients[fd]);
	}

	chan.broadcast(":" + this->_clients[fd].getPrefix() + " JOIN " + chan_name, NULL);

	if (!chan.getTopic().empty())
	{
		this->send_reply(fd, this->rpl("332", fd) + " " + chan_name + " :" + chan.getTopic());
	}

	this->send_reply(fd, this->rpl("353", fd) + " = " + chan_name + " :" + this->build_names_list(chan));
	this->send_reply(fd, this->rpl("366", fd) + " " + chan_name + " :End of /NAMES list");
}

void Server::cmd_part(int fd, Message &msg)
{
	if (msg.getParams().empty())
	{
		this->send_reply(fd, this->rpl("461", fd) + " PART :Not enough parameters");
		return ;
	}

	std::string chan_name = msg.getParams()[0];

	if (this->_channels.find(chan_name) == this->_channels.end())
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	Channel &chan = this->_channels[chan_name];

	if (!chan.isMember(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("442", fd) + " " + chan_name + " :You're not on that channel");
		return ;
	}

	std::string reason = "Leaving";

	if (msg.getParams().size() > 1)
	{
		reason = msg.getParams()[1];
	}

	chan.broadcast(":" + this->_clients[fd].getPrefix() + " PART " + chan_name + " :" + reason, NULL);
	chan.removeClient(&this->_clients[fd]);
	if (chan.getMemberCount() == 0)
	{
		this->_channels.erase(chan_name);
	}
}

/* ------------------------------------------------------------------------ */
/* PRIVMSG                                                                   */
/* ------------------------------------------------------------------------ */

void Server::cmd_privmsg(int fd, Message &msg)
{
	if (msg.getParams().empty())
	{
		this->send_reply(fd, this->rpl("411", fd) + " :No recipient given (PRIVMSG)");
		return ;
	}

	if (msg.getParams().size() < 2)
	{
		this->send_reply(fd, this->rpl("412", fd) + " :No text to send");
		return ;
	}

	std::string target = msg.getParams()[0];
	std::string text = msg.getParams()[1];
	std::string prefix = ":" + this->_clients[fd].getPrefix();

	if (target[0] == '#')
	{
		if (this->_channels.find(target) == this->_channels.end())
		{
			this->send_reply(fd, this->rpl("403", fd) + " " + target + " :No such channel");
			return ;
		}

		Channel &chan = this->_channels[target];

		if (!chan.isMember(&this->_clients[fd]))
		{
			this->send_reply(fd, this->rpl("404", fd) + " " + target + " :Cannot send to channel");
			return ;
		}

		chan.broadcast(prefix + " PRIVMSG " + target + " :" + text, &this->_clients[fd]);
	}
	else
	{
		Client *dest = this->find_client_by_nick(target);

		if (!dest)
		{
			this->send_reply(fd, this->rpl("401", fd) + " " + target + " :No such nick/channel");
			return ;
		}

		this->send_reply(dest->get_fd(), prefix + " PRIVMSG " + target + " :" + text);
	}
}

/* ------------------------------------------------------------------------ */
/* QUIT / PING                                                               */
/* ------------------------------------------------------------------------ */

void Server::cmd_quit(int fd, Message &msg, size_t &index)
{
	std::string reason = "Client quit";

	if (!msg.getParams().empty())
	{
		reason = msg.getParams()[0];
	}

	this->remove_client(fd, reason);
	close(fd);
	this->_clients.erase(fd);
	this->_pollfds.erase(this->_pollfds.begin() + index);
	index--;
}

void Server::cmd_ping(int fd, Message &msg)
{
	std::string token;

	if (!msg.getParams().empty())
	{
		token = msg.getParams()[0];
	}

	this->send_reply(fd, ":" + this->_hostname + " PONG " + this->_hostname + " :" + token);
}

/* ------------------------------------------------------------------------ */
/* KICK / INVITE / TOPIC                                                    */
/* ------------------------------------------------------------------------ */

void Server::cmd_kick(int fd, Message &msg)
{
	if (msg.getParams().size() < 2)
	{
		this->send_reply(fd, this->rpl("461", fd) + " KICK :Not enough parameters");
		return ;
	}

	std::string chan_name = msg.getParams()[0];
	std::string target_nick = msg.getParams()[1];
	std::string reason = this->_clients[fd].get_nickname();

	if (msg.getParams().size() > 2)
	{
		reason = msg.getParams()[2];
	}

	if (this->_channels.find(chan_name) == this->_channels.end())
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	Channel &chan = this->_channels[chan_name];

	if (!chan.isMember(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("442", fd) + " " + chan_name + " :You're not on that channel");
		return ;
	}

	if (!chan.isOperator(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("482", fd) + " " + chan_name + " :You're not channel operator");
		return ;
	}

	Client *target = this->find_client_by_nick(target_nick);

	if (!target || !chan.isMember(target))
	{
		this->send_reply(fd, this->rpl("441", fd) + " " + target_nick + " " + chan_name + " :They aren't on that channel");
		return ;
	}

	chan.broadcast(":" + this->_clients[fd].getPrefix() + " KICK " + chan_name + " " + target_nick + " :" + reason, NULL);
	chan.removeClient(target);
	if (chan.getMemberCount() == 0)
	{
		this->_channels.erase(chan_name);
	}
}

void Server::cmd_invite(int fd, Message &msg)
{
	if (msg.getParams().size() < 2)
	{
		this->send_reply(fd, this->rpl("461", fd) + " INVITE :Not enough parameters");
		return ;
	}

	std::string target_nick = msg.getParams()[0];
	std::string chan_name = msg.getParams()[1];

	if (this->_channels.find(chan_name) == this->_channels.end())
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	Channel &chan = this->_channels[chan_name];

	if (!chan.isMember(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("442", fd) + " " + chan_name + " :You're not on that channel");
		return ;
	}

	if (chan.isInviteOnly() && !chan.isOperator(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("482", fd) + " " + chan_name + " :You're not channel operator");
		return ;
	}

	Client *target = this->find_client_by_nick(target_nick);

	if (!target)
	{
		this->send_reply(fd, this->rpl("401", fd) + " " + target_nick + " :No such nick/channel");
		return ;
	}

	if (chan.isMember(target))
	{
		this->send_reply(fd, this->rpl("443", fd) + " " + target_nick + " " + chan_name + " :is already on channel");
		return ;
	}

	chan.addInvite(target_nick);
	this->send_reply(fd, this->rpl("341", fd) + " " + target_nick + " " + chan_name);
	this->send_reply(target->get_fd(), ":" + this->_clients[fd].getPrefix() + " INVITE " + target_nick + " " + chan_name);
}

void Server::cmd_topic(int fd, Message &msg)
{
	if (msg.getParams().empty())
	{
		this->send_reply(fd, this->rpl("461", fd) + " TOPIC :Not enough parameters");
		return ;
	}

	std::string chan_name = msg.getParams()[0];

	if (this->_channels.find(chan_name) == this->_channels.end())
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	Channel &chan = this->_channels[chan_name];

	if (!chan.isMember(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("442", fd) + " " + chan_name + " :You're not on that channel");
		return ;
	}

	if (msg.getParams().size() < 2)
	{
		if (chan.getTopic().empty())
		{
			this->send_reply(fd, this->rpl("331", fd) + " " + chan_name + " :No topic is set");
		}
		else
		{
			this->send_reply(fd, this->rpl("332", fd) + " " + chan_name + " :" + chan.getTopic());
		}
		return ;
	}

	if (chan.isTopicRestricted() && !chan.isOperator(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("482", fd) + " " + chan_name + " :You're not channel operator");
		return ;
	}

	chan.setTopic(msg.getParams()[1]);
	chan.broadcast(":" + this->_clients[fd].getPrefix() + " TOPIC " + chan_name + " :" + msg.getParams()[1], NULL);
}

/* ------------------------------------------------------------------------ */
/* MODE                                                                      */
/* ------------------------------------------------------------------------ */

void Server::cmd_mode(int fd, Message &msg)
{
	if (msg.getParams().empty())
	{
		this->send_reply(fd, this->rpl("461", fd) + " MODE :Not enough parameters");
		return ;
	}

	std::string chan_name = msg.getParams()[0];

	if (chan_name.empty() || chan_name[0] != '#')
	{
		return ;
	}

	if (this->_channels.find(chan_name) == this->_channels.end())
	{
		this->send_reply(fd, this->rpl("403", fd) + " " + chan_name + " :No such channel");
		return ;
	}

	Channel &chan = this->_channels[chan_name];

	if (msg.getParams().size() < 2)
	{
		std::string modes = "+";
		std::string mparams;

		if (chan.isInviteOnly())
		{
			modes += "i";
		}

		if (chan.isTopicRestricted())
		{
			modes += "t";
		}

		if (!chan.getKey().empty())
		{
			modes += "k";
			mparams += " " + chan.getKey();
		}

		if (chan.getUserLimit() > 0)
		{
			std::ostringstream oss;

			modes += "l";
			oss << chan.getUserLimit();
			mparams += " " + oss.str();
		}

		if (modes == "+")
		{
			modes.clear();
		}

		this->send_reply(fd, this->rpl("324", fd) + " " + chan_name + " " + modes + mparams);
		return ;
	}

	if (!chan.isOperator(&this->_clients[fd]))
	{
		this->send_reply(fd, this->rpl("482", fd) + " " + chan_name + " :You're not channel operator");
		return ;
	}

	std::string mode_str = msg.getParams()[1];
	bool adding = true;
	size_t pi = 2;
	std::string applied_modes;
	std::string applied_params;
	bool cur_sign_plus = true;
	bool need_sign = true;

	for (size_t i = 0; i < mode_str.size(); i++)
	{
		char c = mode_str[i];

		if (c == '+')
		{
			adding = true;
			need_sign = true;
			continue ;
		}

		if (c == '-')
		{
			adding = false;
			need_sign = true;
			continue ;
		}

		bool ok = false;
		std::string param;

		if (c == 'i')
		{
			chan.setInviteOnly(adding);
			ok = true;
		}
		else if (c == 't')
		{
			chan.setTopicRestricted(adding);
			ok = true;
		}
		else if (c == 'k')
		{
			if (adding)
			{
				if (pi >= msg.getParams().size())
				{
					this->send_reply(fd, this->rpl("461", fd) + " MODE :Not enough parameters");
					continue ;
				}

				param = msg.getParams()[pi];
				pi++;
				chan.setKey(param);
			}
			else
			{
				chan.setKey("");
			}
			ok = true;
		}
		else if (c == 'o')
		{
			if (pi >= msg.getParams().size())
			{
				this->send_reply(fd, this->rpl("461", fd) + " MODE :Not enough parameters");
				continue ;
			}

			param = msg.getParams()[pi];
			pi++;

			Client *t = this->find_client_by_nick(param);

			if (!t || !chan.isMember(t))
			{
				this->send_reply(fd, this->rpl("441", fd) + " " + param + " " + chan_name + " :They aren't on that channel");
				continue ;
			}

			if (adding)
			{
				chan.giveOperator(t);
			}
			else
			{
				chan.removeOperator(t);
			}
			ok = true;
		}
		else if (c == 'l')
		{
			if (adding)
			{
				if (pi >= msg.getParams().size())
				{
					this->send_reply(fd, this->rpl("461", fd) + " MODE :Not enough parameters");
					continue ;
				}

				int limit = std::atoi(msg.getParams()[pi].c_str());

				pi++;
				if (limit <= 0)
				{
					this->send_reply(fd, this->rpl("461", fd) + " MODE :Invalid limit");
					continue ;
				}

				chan.setUserLimit(limit);

				std::ostringstream oss;

				oss << limit;
				param = oss.str();
			}
			else
			{
				chan.setUserLimit(0);
			}
			ok = true;
		}
		else
		{
			this->send_reply(fd, this->rpl("472", fd) + " " + std::string(1, c) + " :is unknown mode char to me");
			continue ;
		}

		if (ok)
		{
			if (need_sign || cur_sign_plus != adding)
			{
				if (adding)
				{
					applied_modes += '+';
				}
				else
				{
					applied_modes += '-';
				}

				cur_sign_plus = adding;
				need_sign = false;
			}

			applied_modes += c;
			if (!param.empty())
			{
				applied_params += " " + param;
			}
		}
	}

	if (!applied_modes.empty())
	{
		chan.broadcast(":" + this->_clients[fd].getPrefix() + " MODE " + chan_name + " " + applied_modes + applied_params, NULL);
	}
}

/* ------------------------------------------------------------------------ */
/* Command dispatch                                                          */
/* ------------------------------------------------------------------------ */

void Server::dispatch_cmd(int fd, Message &msg, size_t &index)
{
	std::string cmd = msg.getCmd();

	for (size_t i = 0; i < cmd.size(); i++)
	{
		cmd[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd[i])));
	}

	if (cmd == "PASS")
	{
		this->cmd_pass(fd, msg);
	}
	else if (cmd == "NICK")
	{
		this->cmd_nick(fd, msg);
	}
	else if (cmd == "USER")
	{
		this->cmd_user(fd, msg);
	}
	else if (cmd == "PING")
	{
		this->cmd_ping(fd, msg);
	}
	else if (!this->_clients[fd].get_is_registered())
	{
		this->send_reply(fd, ":" + this->_hostname + " 451 * :You have not registered");
	}
	else if (cmd == "JOIN")
	{
		this->cmd_join(fd, msg);
	}
	else if (cmd == "PART")
	{
		this->cmd_part(fd, msg);
	}
	else if (cmd == "PRIVMSG")
	{
		this->cmd_privmsg(fd, msg);
	}
	else if (cmd == "QUIT")
	{
		this->cmd_quit(fd, msg, index);
	}
	else if (cmd == "KICK")
	{
		this->cmd_kick(fd, msg);
	}
	else if (cmd == "INVITE")
	{
		this->cmd_invite(fd, msg);
	}
	else if (cmd == "TOPIC")
	{
		this->cmd_topic(fd, msg);
	}
	else if (cmd == "MODE")
	{
		this->cmd_mode(fd, msg);
	}
	else
	{
		this->send_reply(fd, this->rpl("421", fd) + " " + cmd + " :Unknown command");
	}
}

/* ------------------------------------------------------------------------ */
/* Network I/O                                                              */
/* ------------------------------------------------------------------------ */

void Server::add_new_client()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(this->_srv_socket, (struct sockaddr *)&client_addr, &client_len);

	if (client_fd == -1)
	{
		std::cerr << "Error: could not accept user." << std::endl;
		return ;
	}

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: fcntl failed." << std::endl;
		close(client_fd);
		return ;
	}

	std::string ip = inet_ntoa(client_addr.sin_addr);

	std::cout << "Client connected fd:" << client_fd << " ip:" << ip << std::endl;

	struct pollfd pfd;

	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	this->_pollfds.push_back(pfd);
	this->_clients[client_fd] = Client(client_fd, ip);
}

void Server::parse_client_message(size_t &index)
{
	char buffer[1024];
	Message msg;
	int fd = this->_pollfds[index].fd;
	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes <= 0)
	{
		std::cout << "Client " << fd << " disconnected." << std::endl;
		this->remove_client(fd, "Client disconnected");
		close(fd);
		this->_clients.erase(fd);
		this->_pollfds.erase(this->_pollfds.begin() + index);
		index--;
		return ;
	}

	buffer[bytes] = '\0';
	this->_clients[fd].push_back_buf(buffer);

	std::string &buf = this->_clients[fd].get_recv_buf();
	size_t pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		std::string line = buf.substr(0, pos);

		buf.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
		{
			line.erase(line.size() - 1);
		}

		if (!line.empty())
		{
			msg.parse(line);
			this->dispatch_cmd(fd, msg, index);
			if (this->_clients.find(fd) == this->_clients.end())
			{
				return ;
			}
		}
	}
}

void Server::run()
{
	std::cout << "Starting server..." << std::endl;
	this->_srv_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (this->_srv_socket == -1)
	{
		std::cerr << "Error: could not create socket" << std::endl;
		return ;
	}

	int opt = 1;

	if (setsockopt(this->_srv_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error: could not set socket options" << std::endl;
		close(this->_srv_socket);
		return ;
	}

	if (fcntl(this->_srv_socket, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: could not set non-blocking mode" << std::endl;
		close(this->_srv_socket);
		return ;
	}

	struct sockaddr_in serv;

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(this->_port);
	serv.sin_addr.s_addr = INADDR_ANY;

	if (bind(this->_srv_socket, (struct sockaddr *)&serv, sizeof(serv)) < 0)
	{
		std::cerr << "Error: binding socket" << std::endl;
		close(this->_srv_socket);
		return ;
	}

	if (listen(this->_srv_socket, SOMAXCONN) < 0)
	{
		std::cerr << "Error: listening on socket" << std::endl;
		close(this->_srv_socket);
		return ;
	}

	std::cout << "IRC Server is online on port " << this->_port << std::endl;

	struct pollfd pfd;

	pfd.fd = this->_srv_socket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	this->_pollfds.push_back(pfd);

	while (g_is_running)
	{
		if (poll(&this->_pollfds[0], this->_pollfds.size(), -1) < 0)
		{
			if (!g_is_running)
			{
				break ;
			}

			std::cerr << "Error: poll() failed" << std::endl;
			break ;
		}

		for (size_t i = 0; i < this->_pollfds.size(); i++)
		{
			int fd = this->_pollfds[i].fd;
			short revents = this->_pollfds[i].revents;

			if (revents == 0)
			{
				continue ;
			}

			if (fd != this->_srv_socket && (revents & (POLLERR | POLLHUP)))
			{
				std::cout << "Client " << fd << " disconnected." << std::endl;
				this->remove_client(fd, "Connection lost");
				close(fd);
				this->_clients.erase(fd);
				this->_pollfds.erase(this->_pollfds.begin() + i);
				i--;
				continue ;
			}

			if (revents & POLLIN)
			{
				if (fd == this->_srv_socket)
				{
					this->add_new_client();
				}
				else
				{
					this->parse_client_message(i);
				}
			}

			if (fd != this->_srv_socket && (revents & POLLOUT) && this->_clients.find(fd) != this->_clients.end())
			{
				this->flush_client(fd);
			}
		}

		this->sync_poll_events();
	}
}

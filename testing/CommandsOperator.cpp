#include "Server.hpp"

/* ------------------------------------------------------------------------ */
/* KICK                                                                      */
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

/* ------------------------------------------------------------------------ */
/* INVITE                                                                    */
/* ------------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------------ */
/* TOPIC                                                                     */
/* ------------------------------------------------------------------------ */

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

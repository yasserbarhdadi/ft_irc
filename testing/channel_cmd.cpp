#include "Server.hpp"

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

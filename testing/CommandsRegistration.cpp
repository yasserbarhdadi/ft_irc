#include "Server.hpp"

/* ------------------------------------------------------------------------ */
/* PASS                                                                      */
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

/* ------------------------------------------------------------------------ */
/* NICK                                                                      */
/* ------------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------------ */
/* USER                                                                      */
/* ------------------------------------------------------------------------ */

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
/* PING                                                                      */
/* ------------------------------------------------------------------------ */

void Server::cmd_ping(int fd, Message &msg)
{
	std::string token;

	if (!msg.getParams().empty())
	{
		token = msg.getParams()[0];
	}

	this->send_reply(fd, ":" + this->_hostname + " PONG " + this->_hostname + " :" + token);
}

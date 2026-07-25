#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel()
	: _inviteOnly(false), _topicRestricted(false), _userLimit(0)
{
}

Channel::Channel(const std::string &name)
	: _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0)
{
}

Channel::Channel(const Channel &o)
	: _name(o._name), _topic(o._topic), _key(o._key),
	  _inviteOnly(o._inviteOnly), _topicRestricted(o._topicRestricted),
	  _userLimit(o._userLimit), _members(o._members),
	  _operators(o._operators), _invited(o._invited)
{
}

Channel &Channel::operator=(const Channel &o)
{
	if (this != &o)
	{
		this->_name = o._name;
		this->_topic = o._topic;
		this->_key = o._key;
		this->_inviteOnly = o._inviteOnly;
		this->_topicRestricted = o._topicRestricted;
		this->_userLimit = o._userLimit;
		this->_members = o._members;
		this->_operators = o._operators;
		this->_invited = o._invited;
	}
	return (*this);
}

Channel::~Channel()
{
}

void Channel::addClient(Client *client)
{
	if (!client)
	{
		return ;
	}

	if (std::find(this->_members.begin(), this->_members.end(), client) == this->_members.end())
	{
		this->_members.push_back(client);
	}
}

void Channel::removeClient(Client *client)
{
	std::vector<Client*>::iterator it;

	it = std::find(this->_members.begin(), this->_members.end(), client);
	if (it != this->_members.end())
	{
		this->_members.erase(it);
	}

	it = std::find(this->_operators.begin(), this->_operators.end(), client);
	if (it != this->_operators.end())
	{
		this->_operators.erase(it);
	}
}

bool Channel::isMember(Client *client) const
{
	return (std::find(this->_members.begin(), this->_members.end(), client) != this->_members.end());
}

int Channel::getMemberCount() const
{
	return (static_cast<int>(this->_members.size()));
}

std::vector<Client*> Channel::getMembers() const
{
	return (this->_members);
}

void Channel::giveOperator(Client *client)
{
	if (!client || !this->isMember(client))
	{
		return ;
	}

	if (std::find(this->_operators.begin(), this->_operators.end(), client) == this->_operators.end())
	{
		this->_operators.push_back(client);
	}
}

void Channel::removeOperator(Client *client)
{
	std::vector<Client*>::iterator it = std::find(this->_operators.begin(), this->_operators.end(), client);

	if (it != this->_operators.end())
	{
		this->_operators.erase(it);
	}
}

bool Channel::isOperator(Client *client) const
{
	return (std::find(this->_operators.begin(), this->_operators.end(), client) != this->_operators.end());
}

std::string Channel::getName() const
{
	return (this->_name);
}

std::string Channel::getTopic() const
{
	return (this->_topic);
}

void Channel::setTopic(const std::string &topic)
{
	this->_topic = topic;
}

bool Channel::isInviteOnly() const
{
	return (this->_inviteOnly);
}

void Channel::setInviteOnly(bool v)
{
	this->_inviteOnly = v;
}

bool Channel::isTopicRestricted() const
{
	return (this->_topicRestricted);
}

void Channel::setTopicRestricted(bool v)
{
	this->_topicRestricted = v;
}

std::string Channel::getKey() const
{
	return (this->_key);
}

void Channel::setKey(const std::string &key)
{
	this->_key = key;
}

int Channel::getUserLimit() const
{
	return (this->_userLimit);
}

void Channel::setUserLimit(int limit)
{
	this->_userLimit = limit;
}

void Channel::addInvite(const std::string &nick)
{
	if (std::find(this->_invited.begin(), this->_invited.end(), nick) == this->_invited.end())
	{
		this->_invited.push_back(nick);
	}
}

bool Channel::isInvited(const std::string &nick) const
{
	return (std::find(this->_invited.begin(), this->_invited.end(), nick) != this->_invited.end());
}

void Channel::removeInvite(const std::string &nick)
{
	std::vector<std::string>::iterator it = std::find(this->_invited.begin(), this->_invited.end(), nick);

	if (it != this->_invited.end())
	{
		this->_invited.erase(it);
	}
}

void Channel::broadcast(const std::string &msg, Client *exclude)
{
	std::string full = msg + "\r\n";

	for (size_t i = 0; i < this->_members.size(); i++)
	{
		if (this->_members[i] != exclude)
		{
			this->_members[i]->queue_msg(full);
		}
	}
}

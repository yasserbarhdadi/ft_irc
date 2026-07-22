#include "Channel.hpp"


Channel::Channel()
{
}

Channel::~Channel()
{
}
//those functions is cheking if the user is already joined the chanels or not in ordder give permissions 


void Channel::addClient(Client *client)
{
	if (client == NULL)
	{
		return ;
	}
	if (std::find(this->members.begin(), this->members.end(), client) == this->members.end())
	{
		this->members.push_back(client);
	}
}

bool Channel::isMember(Client *client) const
{
	return (std::find(this->members.begin(), this->members.end(), client) != this->members.end());
}

void Channel::giveOperator(Client *client)
{
	if (client == NULL || !this->isMember(client))
	{
		return ;
	}
	if (std::find(this->operators.begin(), this->operators.end(), client) == this->operators.end())
	{
		this->operators.push_back(client);
	}
}

void Channel::removeOperator(Client *client)
{
	std::vector<Client*>::iterator it = std::find(this->operators.begin(), this->operators.end(), client);

	if (it != this->operators.end())
	{
		this->operators.erase(it);
	}
}

bool Channel::isOperator(Client *client) const
{
	return (std::find(this->operators.begin(), this->operators.end(), client) != this->operators.end());
}
#pragma once

#include "main.hpp"

class Client;

class Channel
{
	private:
		std::vector<Client*>	members;
		std::vector<Client*>	operators;

	public:
		Channel();
		~Channel();

		void	addClient(Client *client);
		bool	isMember(Client *client) const;

		void	giveOperator(Client *client);
		void	removeOperator(Client *client);
		bool	isOperator(Client *client) const;
};
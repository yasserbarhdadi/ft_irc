#include "Message.hpp"

Message::Message()
{
}

Message::~Message()
{
}

std::string Message::getCmd() const
{
	return (this->cmd);
}

std::vector<std::string> Message::getParams() const
{
	return (this->params);
}

void Message::parse(std::string &ln)
{
	this->cmd.clear();
	this->params.clear();

	if (!ln.empty() && ln[0] == ':')
	{
		size_t sp = ln.find(' ');

		if (sp == std::string::npos)
		{
			return ;
		}
		ln = ln.substr(sp + 1);
	}

	while (!ln.empty() && ln[0] == ' ')
	{
		ln.erase(0, 1);
	}

	if (ln.empty())
	{
		return ;
	}

	size_t sp = ln.find(' ');

	if (sp == std::string::npos)
	{
		this->cmd = ln;
		return ;
	}

	this->cmd = ln.substr(0, sp);
	ln = ln.substr(sp + 1);

	while (!ln.empty())
	{
		while (!ln.empty() && ln[0] == ' ')
		{
			ln.erase(0, 1);
		}

		if (ln.empty())
		{
			break ;
		}

		if (ln[0] == ':')
		{
			this->params.push_back(ln.substr(1));
			break ;
		}

		sp = ln.find(' ');
		if (sp == std::string::npos)
		{
			this->params.push_back(ln);
			break ;
		}

		this->params.push_back(ln.substr(0, sp));
		ln = ln.substr(sp + 1);
	}
}

#include "Message.hpp"

std::string Message::getCmd() const
{
	return cmd;
}

std::vector<std::string> Message::getParams() const
{
	return params;
}

void Message::parse(std::string &ln)
{
	cmd.clear();
	params.clear();

<<<<<<< HEAD
	if (!ln.empty() && ln[0] == ':') {
		space_index = ln.find(' ');
		if (space_index == std::string::npos)
=======
	if (!ln.empty() && ln[0] == ':')
	{
		size_t sp = ln.find(' ');
		if (sp == std::string::npos)
>>>>>>> 7eaaa62 (completed commands)
			return ;
		ln = ln.substr(sp + 1);
	}
	while (!ln.empty() && ln[0] == ' ')
		ln.erase(0, 1);
	if (ln.empty())
		return ;

	size_t sp = ln.find(' ');
	if (sp == std::string::npos)
	{
		cmd = ln;
		return ;
	}
	cmd = ln.substr(0, sp);
	ln = ln.substr(sp + 1);

	while (!ln.empty())
	{
		while (!ln.empty() && ln[0] == ' ')
			ln.erase(0, 1);
		if (ln.empty())
			break ;
		if (ln[0] == ':')
		{
			params.push_back(ln.substr(1));
			break ;
		}
		sp = ln.find(' ');
		if (sp == std::string::npos)
		{
			params.push_back(ln);
			break ;
		}
		params.push_back(ln.substr(0, sp));
		ln = ln.substr(sp + 1);
	}
}

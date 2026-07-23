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
	size_t space_index;

	if (ln[0] == ':') {
		space_index = ln.find(' ');
		if (space_index == std::string::npos)
			return ;
		ln = ln.substr(space_index + 1);
	}

	while (!ln.empty() && ln[0] == ' ')
		ln.erase(0, 1);

	if (ln.empty())
		return ;
	
	space_index = ln.find(' ');
	if (space_index == std::string::npos) {
		cmd = ln;
		return ;
	}
	else {
		cmd = ln.substr(0, space_index);
		ln = ln.substr(space_index + 1);
	}

	while (!ln.empty())
	{
		while (!ln.empty() && ln[0] == ' ')
			ln.erase(0, 1);
		
		if (ln.empty())
			break;
		
		if (ln[0] == ':') {
			params.push_back(ln.substr(1));
			break;
		}

		space_index = ln.find(' ');
		if (space_index == std::string::npos) {
			params.push_back(ln);
			break;
		}
		else {
			params.push_back(ln.substr(0, space_index));
			ln = ln.substr(space_index + 1);
		}
	}
}

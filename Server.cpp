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

void Server::dispatch_cmd(int fd, Message &msg)
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
		shutdown(fd, SHUT_RDWR);
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
			this->dispatch_cmd(fd, msg);
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

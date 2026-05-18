/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yabarhda <yabarhda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 13:25:22 by yabarhda          #+#    #+#             */
/*   Updated: 2026/05/18 21:23:52 by yabarhda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.hpp"
#include "Client.hpp"
#include "Message.hpp"

#include <sstream>

static std::vector<Client>::iterator find_client(std::vector<Client> &clients, int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->get_fd() == fd)
			return it;
	}
	return clients.end();
}

static std::string strip_line_endings(std::string line)
{
	while (!line.empty() && (line[line.size() - 1] == '\n' || line[line.size() - 1] == '\r'))
		line.erase(line.size() - 1);
	return line;
}

static void parse_message(const std::string &line, Message &message)
{
	std::stringstream ss(line);
	std::string token;

	message.clear();
	if (ss >> token)
	{
		message.set_cmd(token);
		while (ss >> token)
			message.add_av(token);
	}
}

static void send_text(int fd, const std::string &text)
{
	if (send(fd, text.c_str(), text.size(), 0) < 0)
		std::cerr << "Error: send failed on FD " << fd << std::endl;
}

static void disconnect_client(std::vector<struct pollfd> &fds, std::vector<Client> &clients, size_t index)
{
	int fd = fds[index].fd;
	close(fd);
	fds.erase(fds.begin() + index);
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->get_fd() == fd)
		{
			clients.erase(it);
			break;
		}
	}
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "Error: socket creation failed" << std::endl;
		return 1;
	}

	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error: setsockopt failed" << std::endl;
		close(sock);
		return 1;
	}

	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl failed on master socket" << std::endl;
		close(sock);
		return 1;
	}

	struct sockaddr_in socketaddr;
	std::memset(&socketaddr, 0, sizeof(socketaddr));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(std::atoi(av[1]));
	socketaddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&socketaddr, sizeof(socketaddr)) < 0)
	{
		std::cerr << "Error: bind failed" << std::endl;
		close(sock);
		return 1;
	}

	if (listen(sock, 32) < 0)
	{
		std::cerr << "Error: listen failed" << std::endl;
		close(sock);
		return 1;
	}

	std::cout << "Server listening on port " << av[1] << std::endl;

	std::vector<struct pollfd> fds;
	std::vector<Client> clients;
	struct pollfd master_pollfd;
	master_pollfd.fd = sock;
	master_pollfd.events = POLLIN;
	master_pollfd.revents = 0;
	fds.push_back(master_pollfd);

	while (true)
	{
		int poll_count = poll(&fds[0], fds.size(), -1);
		if (poll_count < 0)
		{
			std::cerr << "Error: poll failed" << std::endl;
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (!(fds[i].revents & POLLIN))
				continue;

			if (fds[i].fd == sock)
			{
				struct sockaddr_in clientAddr;
				socklen_t clientSize = sizeof(clientAddr);
				int clientFd = accept(sock, (struct sockaddr *)&clientAddr, &clientSize);
				if (clientFd < 0)
				{
					std::cerr << "Error: accept failed" << std::endl;
					continue;
				}
				if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
				{
					std::cerr << "Error: fcntl failed on client socket" << std::endl;
					close(clientFd);
					continue;
				}
				struct pollfd client_pollfd;
				client_pollfd.fd = clientFd;
				client_pollfd.events = POLLIN;
				client_pollfd.revents = 0;
				fds.push_back(client_pollfd);
				clients.push_back(Client(clientFd));
				std::cout << "New client connected on FD: " << clientFd << std::endl;
			}
			else
			{
				char buffer[1024];
				std::memset(buffer, 0, sizeof(buffer));
				int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes_received <= 0)
				{
					std::cout << "Client on FD " << fds[i].fd << " disconnected" << std::endl;
					disconnect_client(fds, clients, i);
					--i;
					continue;
				}

				std::vector<Client>::iterator client_it = find_client(clients, fds[i].fd);
				if (client_it == clients.end())
					continue;

				client_it->append_to_buffer(std::string(buffer, bytes_received));
				std::string full_buffer = client_it->get_buffer();
				std::string::size_type newline_pos = full_buffer.find('\n');

				while (newline_pos != std::string::npos)
				{
					std::string line = strip_line_endings(full_buffer.substr(0, newline_pos));
					full_buffer.erase(0, newline_pos + 1);

					Message message;
					parse_message(line, message);

					if (message.get_cmd() == "PASS")
					{
						if (!message.get_av().empty() && message.get_av()[0] == av[2])
						{
							client_it->set_registered(true);
							send_text(fds[i].fd, ":server 001 Password accepted\r\n");
						}
						else
						{
							send_text(fds[i].fd, ":server 464 Password incorrect\r\n");
						}
					}
					else if (!client_it->get_registered())
					{
						send_text(fds[i].fd, ":server 451 You have not registered\r\n");
					}
					else if (!message.get_cmd().empty())
					{
						send_text(fds[i].fd, ":server 200 " + message.get_cmd() + " received\r\n");
					}

					newline_pos = full_buffer.find('\n');
				}

				client_it->remove_from_buffer(client_it->get_buffer().size());
				client_it->append_to_buffer(full_buffer);
			}
		}
	}

	for (size_t i = 0; i < fds.size(); ++i)
		close(fds[i].fd);
	return 0;
}

#pragma once

#include "main.hpp"

#define SERVER_NAME "ircserv"

class Server
{
	private:
		std::string                      _password;
		int                               _port;
		int                               _srv_socket;
		std::string                       _hostname;
		std::vector<struct pollfd>        _pollfds;
		std::map<int, Client>              _clients;
		std::map<std::string, Channel>    _channels;

		/* output: nothing is ever written straight to a socket.
		 * send_reply() and Channel::broadcast() only queue data on
		 * the target Client; flush_client() is the single place
		 * that actually calls send(), and it is only called once
		 * poll() reports the fd as writable. */
		void    send_reply(int fd, const std::string &reply);
		void    flush_client(int fd);
		void    sync_poll_events();

		void    cmd_pass(int fd, Message &msg);
		void    cmd_nick(int fd, Message &msg);
		void    cmd_user(int fd, Message &msg);
		void    cmd_join(int fd, Message &msg);
		void    cmd_part(int fd, Message &msg);
		void    cmd_privmsg(int fd, Message &msg);
		void    cmd_quit(int fd, Message &msg, size_t &index);
		void    cmd_kick(int fd, Message &msg);
		void    cmd_invite(int fd, Message &msg);
		void    cmd_topic(int fd, Message &msg);
		void    cmd_mode(int fd, Message &msg);
		void    cmd_ping(int fd, Message &msg);

		void        remove_client(int fd, const std::string &reason);
		Client      *find_client_by_nick(const std::string &nick);
		std::string  build_names_list(Channel &chan);
		std::string  rpl(const std::string &code, int fd);

	public:
		Server();
		~Server();
		Server(const Server &other);
		Server(std::string passwd, int prt);
		Server &operator=(const Server &other);

		void run();
		void add_new_client();
		void parse_client_message(size_t &index);
		void dispatch_cmd(int fd, Message &msg, size_t &index);
};

extern volatile sig_atomic_t g_is_running;

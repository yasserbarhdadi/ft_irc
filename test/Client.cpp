#include "Client.hpp"

Client::Client()
	: fd(-1), is_registered(false), is_pass(false), is_nick(false), is_user(false)
{
}

Client::Client(int fd, std::string ip)
	: fd(fd), ip_address(ip), is_registered(false), is_pass(false), is_nick(false), is_user(false)
{
}

Client::~Client()
{
}

Client::Client(const Client &o)
	: fd(o.fd), ip_address(o.ip_address), nickname(o.nickname),
	  username(o.username), realname(o.realname), recv_buffer(o.recv_buffer),
	  send_buffer(o.send_buffer), is_registered(o.is_registered), is_pass(o.is_pass),
	  is_nick(o.is_nick), is_user(o.is_user)
{
}

Client &Client::operator=(const Client &o)
{
	if (this != &o)
	{
		this->fd = o.fd;
		this->ip_address = o.ip_address;
		this->nickname = o.nickname;
		this->username = o.username;
		this->realname = o.realname;
		this->recv_buffer = o.recv_buffer;
		this->send_buffer = o.send_buffer;
		this->is_registered = o.is_registered;
		this->is_pass = o.is_pass;
		this->is_nick = o.is_nick;
		this->is_user = o.is_user;
	}
	return (*this);
}

int Client::get_fd() const
{
	return (this->fd);
}

std::string Client::getPrefix() const
{
	std::string n;
	std::string u;

	if (this->nickname.empty())
	{
		n = "*";
	}
	else
	{
		n = this->nickname;
	}

	if (this->username.empty())
	{
		u = "*";
	}
	else
	{
		u = this->username;
	}

	return (n + "!" + u + "@" + this->ip_address);
}

bool Client::get_is_registered() const
{
	return (this->is_registered);
}

void Client::set_is_registered(bool v)
{
	this->is_registered = v;
}

void Client::push_back_buf(char *buf)
{
	this->recv_buffer += buf;
}

std::string &Client::get_recv_buf()
{
	return (this->recv_buffer);
}

bool Client::get_is_pass() const
{
	return (this->is_pass);
}

void Client::set_is_pass(bool v)
{
	this->is_pass = v;
}

bool Client::get_is_nick() const
{
	return (this->is_nick);
}

void Client::set_is_nick(bool v)
{
	this->is_nick = v;
}

bool Client::get_is_user() const
{
	return (this->is_user);
}

void Client::set_is_user(bool v)
{
	this->is_user = v;
}

bool Client::try_register()
{
	if (!this->is_registered && this->is_pass && this->is_nick && this->is_user)
	{
		this->is_registered = true;
		return (true);
	}
	return (false);
}

std::string Client::get_nickname() const
{
	return (this->nickname);
}

void Client::set_nickname(const std::string &n)
{
	this->nickname = n;
}

std::string Client::get_username() const
{
	return (this->username);
}

void Client::set_username(const std::string &u)
{
	this->username = u;
}

std::string Client::get_realname() const
{
	return (this->realname);
}

void Client::set_realname(const std::string &r)
{
	this->realname = r;
}

void Client::queue_msg(const std::string &data)
{
	this->send_buffer += data;
}

bool Client::has_pending_output() const
{
	return (!this->send_buffer.empty());
}

const std::string &Client::get_send_buf() const
{
	return (this->send_buffer);
}

void Client::consume_send_buf(size_t n)
{
	if (n >= this->send_buffer.size())
	{
		this->send_buffer.clear();
	}
	else
	{
		this->send_buffer.erase(0, n);
	}
}

#include "Client.hpp"

Client::Client()
{
    is_registered = false;
}

Client::Client(int fd)
{
    this->fd = fd;
    is_registered = false;
}

Client::Client(int fd, std::string ip)
{
    this->ip_address = ip;
    this->fd = fd;
    is_registered = false;
}

Client::~Client()
{

}

Client::Client(const Client &obj) :
                fd(obj.fd), ip_address(obj.ip_address), nickname(obj.nickname), username(obj.username), realname(obj.realname), is_registered(obj.is_registered)
{
}

Client::Client(int sock, std::string ip_address, std::string nickname, std::string username, std::string realname, bool is_registered) :
                fd(sock), ip_address(ip_address), nickname(nickname), username(username), realname(realname), is_registered(is_registered)
{

}

Client& Client::operator=(const Client &obj)
{
    if(this != &obj)
    {
        this->fd = obj.fd;
        this->ip_address = obj.ip_address;
        this->nickname = obj.nickname;
        this->username = obj.username;
        this->realname = obj.realname;
        this->is_registered = obj.is_registered;
    }
    return *this;
}

//implemention of the added function


bool Client::get_is_registered() const
{
	return (this->is_registered);
}

void Client::set_is_registered(bool value)
{
	this->is_registered = value;
}

void Client::push_back_buf(char *buf)
{
	recv_buffer += buf;
}

std::string &Client::get_recv_buf()
{
	return recv_buffer;
}

// --- NEW PERMISSION FUNCTIONS ---

bool Client::get_is_pass() const
{
	return (this->is_pass);
}

void Client::set_is_pass(bool value)
{
	this->is_pass = value;
}

bool Client::get_is_nick() const
{
	return (this->is_nick);
}

void Client::set_is_nick(bool value)
{
	this->is_nick = value;
}

bool Client::get_is_user() const
{
	return (this->is_user);
}

void Client::set_is_user(bool value)
{
	this->is_user = value;
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

void Client::set_nickname(const std::string &nick)
{
	this->nickname = nick;
}

std::string Client::get_username() const
{
	return (this->username);
}

void Client::set_username(const std::string &user)
{
	this->username = user;
}

std::string Client::get_realname() const
{
	return (this->realname);
}

void Client::set_realname(const std::string &real)
{
	this->realname = real;
}

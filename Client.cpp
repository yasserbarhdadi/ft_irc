#include "Client.hpp"

Client::Client()
    : fd(-1), is_registered(false), is_pass(false), is_nick(false), is_user(false) {}

Client::Client(int fd)
    : fd(fd), is_registered(false), is_pass(false), is_nick(false), is_user(false) {}

Client::Client(int fd, std::string ip)
    : fd(fd), ip_address(ip), is_registered(false), is_pass(false), is_nick(false), is_user(false) {}

Client::~Client() {}

Client::Client(const Client &o)
    : fd(o.fd), ip_address(o.ip_address), nickname(o.nickname),
      username(o.username), realname(o.realname), recv_buffer(o.recv_buffer),
      is_registered(o.is_registered), is_pass(o.is_pass),
      is_nick(o.is_nick), is_user(o.is_user) {}

Client &Client::operator=(const Client &o)
{
    if (this != &o)
    {
        fd = o.fd;
        ip_address = o.ip_address;
        nickname = o.nickname;
        username = o.username;
        realname = o.realname;
        recv_buffer = o.recv_buffer;
        is_registered = o.is_registered;
        is_pass = o.is_pass;
        is_nick = o.is_nick;
        is_user = o.is_user;
    }
    return *this;
}

int Client::get_fd() const { return fd; }

std::string Client::getPrefix() const
{
    std::string n = nickname.empty() ? "*" : nickname;
    std::string u = username.empty() ? "*" : username;
    return n + "!" + u + "@" + ip_address;
}

bool        Client::get_is_registered() const { return is_registered; }
void        Client::set_is_registered(bool v) { is_registered = v; }
void        Client::push_back_buf(char *buf)  { recv_buffer += buf; }
std::string &Client::get_recv_buf()           { return recv_buffer; }

bool Client::get_is_pass() const { return is_pass; }
void Client::set_is_pass(bool v) { is_pass = v; }
bool Client::get_is_nick() const { return is_nick; }
void Client::set_is_nick(bool v) { is_nick = v; }
bool Client::get_is_user() const { return is_user; }
void Client::set_is_user(bool v) { is_user = v; }

bool Client::try_register()
{
    if (!is_registered && is_pass && is_nick && is_user)
    {
        is_registered = true;
        return true;
    }
    return false;
}

std::string Client::get_nickname() const              { return nickname; }
void        Client::set_nickname(const std::string &n) { nickname = n; }
std::string Client::get_username() const              { return username; }
void        Client::set_username(const std::string &u) { username = u; }
std::string Client::get_realname() const              { return realname; }
void        Client::set_realname(const std::string &r) { realname = r; }

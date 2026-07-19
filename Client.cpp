#include "Client.hpp"

Client::Client()
{

}

Client::~Client()
{

}

Client::Client(const Client &obj) :
                socket(obj.socket), ip_address(obj.ip_address), nickname(obj.nickname), username(obj.username), realname(obj.realname), is_registered(obj.is_registered)
{
}

Client::Client(int sock, std::string ip_address, std::string nickname, std::string username, std::string realname, bool is_registered) :
                socket(sock), ip_address(ip_address), nickname(nickname), username(username), realname(realname), is_registered(is_registered)
{

}

Client& Client::operator=(const Client &obj)
{
    if(this != &obj)
    {
        this->socket = obj.socket;
        this->ip_address = obj.ip_address;
        this->nickname = obj.nickname;
        this->username = obj.username;
        this->realname = obj.realname;
        this->is_registered = obj.is_registered;
    }
    return *this;
}
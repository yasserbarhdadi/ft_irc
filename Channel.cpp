#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel()
    : _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

Channel::Channel(const std::string &name)
    : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

Channel::~Channel() {}

void Channel::addClient(Client *client)
{
    if (!client)
        return ;
    if (std::find(_members.begin(), _members.end(), client) == _members.end())
        _members.push_back(client);
}

void Channel::removeClient(Client *client)
{
    std::vector<Client*>::iterator it;
    it = std::find(_members.begin(), _members.end(), client);
    if (it != _members.end())
        _members.erase(it);
    it = std::find(_operators.begin(), _operators.end(), client);
    if (it != _operators.end())
        _operators.erase(it);
}

bool Channel::isMember(Client *client) const
{
    return std::find(_members.begin(), _members.end(), client) != _members.end();
}

int                  Channel::getMemberCount() const { return _members.size(); }
std::vector<Client*> Channel::getMembers() const     { return _members; }

void Channel::giveOperator(Client *client)
{
    if (!client || !isMember(client))
        return ;
    if (std::find(_operators.begin(), _operators.end(), client) == _operators.end())
        _operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
    std::vector<Client*>::iterator it = std::find(_operators.begin(), _operators.end(), client);
    if (it != _operators.end())
        _operators.erase(it);
}

bool Channel::isOperator(Client *client) const
{
    return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

std::string Channel::getName() const                    { return _name; }
std::string Channel::getTopic() const                   { return _topic; }
void        Channel::setTopic(const std::string &topic) { _topic = topic; }
bool        Channel::isInviteOnly() const               { return _inviteOnly; }
void        Channel::setInviteOnly(bool v)              { _inviteOnly = v; }
bool        Channel::isTopicRestricted() const          { return _topicRestricted; }
void        Channel::setTopicRestricted(bool v)         { _topicRestricted = v; }
std::string Channel::getKey() const                     { return _key; }
void        Channel::setKey(const std::string &key)     { _key = key; }
int         Channel::getUserLimit() const               { return _userLimit; }
void        Channel::setUserLimit(int limit)            { _userLimit = limit; }

void Channel::addInvite(const std::string &nick)
{
    if (std::find(_invited.begin(), _invited.end(), nick) == _invited.end())
        _invited.push_back(nick);
}

bool Channel::isInvited(const std::string &nick) const
{
    return std::find(_invited.begin(), _invited.end(), nick) != _invited.end();
}

void Channel::removeInvite(const std::string &nick)
{
    std::vector<std::string>::iterator it = std::find(_invited.begin(), _invited.end(), nick);
    if (it != _invited.end())
        _invited.erase(it);
}

void Channel::broadcast(const std::string &msg, Client *exclude)
{
    std::string full = msg + "\r\n";
    for (size_t i = 0; i < _members.size(); i++)
    {
        if (_members[i] != exclude)
            send(_members[i]->get_fd(), full.c_str(), full.length(), 0);
    }
}
#pragma once

#include "main.hpp"

class Client;

class Channel
{
    private:
        std::string              _name;
        std::string              _topic;
        std::string              _key;
        bool                     _inviteOnly;
        bool                     _topicRestricted;
        int                      _userLimit;
        std::vector<Client*>     _members;
        std::vector<Client*>     _operators;
        std::vector<std::string> _invited;

    public:
        Channel();
        Channel(const std::string &name);
        ~Channel();

        void                 addClient(Client *client);
        void                 removeClient(Client *client);
        bool                 isMember(Client *client) const;
        int                  getMemberCount() const;
        std::vector<Client*> getMembers() const;

        void                 giveOperator(Client *client);
        void                 removeOperator(Client *client);
        bool                 isOperator(Client *client) const;

        std::string          getName() const;
        std::string          getTopic() const;
        void                 setTopic(const std::string &topic);

        bool                 isInviteOnly() const;
        void                 setInviteOnly(bool value);
        bool                 isTopicRestricted() const;
        void                 setTopicRestricted(bool value);
        std::string          getKey() const;
        void                 setKey(const std::string &key);
        int                  getUserLimit() const;
        void                 setUserLimit(int limit);

        void                 addInvite(const std::string &nick);
        bool                 isInvited(const std::string &nick) const;
        void                 removeInvite(const std::string &nick);

        void                 broadcast(const std::string &msg, Client *exclude);
};
#include "Server.hpp"

Server::Server() : _port(0), _srv_socket(-1) {}

Server::Server(const Server &o)
    : _password(o._password), _port(o._port), _srv_socket(-1), _hostname(o._hostname) {}

Server::Server(std::string passwd, int prt)
    : _password(passwd), _port(prt), _srv_socket(-1), _hostname(SERVER_NAME) {}

Server::~Server()
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        close(it->first);
    if (_srv_socket > 0)
        close(_srv_socket);
}

Server &Server::operator=(const Server &o)
{
    if (this != &o)
    {
        _password = o._password;
        _port = o._port;
        _hostname = o._hostname;
    }
    return *this;
}

void Server::send_reply(int fd, const std::string &reply)
{
    std::string full = reply + "\r\n";
    send(fd, full.c_str(), full.length(), 0);
}

std::string Server::rpl(const std::string &code, int fd)
{
    return ":" + _hostname + " " + code + " " + _clients[fd].get_nickname();
}

Client *Server::find_client_by_nick(const std::string &nick)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.get_nickname() == nick)
            return &it->second;
    }
    return NULL;
}

std::string Server::build_names_list(Channel &chan)
{
    std::string names;
    std::vector<Client*> members = chan.getMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        if (i > 0)
            names += " ";
        if (chan.isOperator(members[i]))
            names += "@";
        names += members[i]->get_nickname();
    }
    return names;
}

void Server::remove_client(int fd)
{
    std::string prefix = ":" + _clients[fd].getPrefix();
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end();)
    {
        if (it->second.isMember(&_clients[fd]))
        {
            it->second.broadcast(prefix + " QUIT :Client disconnected", &_clients[fd]);
            it->second.removeClient(&_clients[fd]);
        }
        if (it->second.getMemberCount() == 0)
            _channels.erase(it++);
        else
            ++it;
    }
}

void Server::cmd_pass(int fd, Message &msg)
{
    if (_clients[fd].get_is_registered())
        return send_reply(fd, ":" + _hostname + " 462 * :You may not reregister");
    if (msg.getParams().empty())
        return send_reply(fd, ":" + _hostname + " 461 * PASS :Not enough parameters");
    if (msg.getParams()[0] == _password)
        _clients[fd].set_is_pass(true);
    else
        send_reply(fd, ":" + _hostname + " 464 * :Password incorrect");
}

void Server::cmd_nick(int fd, Message &msg)
{
    if (!_clients[fd].get_is_pass())
        return send_reply(fd, ":" + _hostname + " 451 * :You have not registered (Send PASS first)");
    if (msg.getParams().empty())
        return send_reply(fd, ":" + _hostname + " 431 * :No nickname given");

    std::string new_nick = msg.getParams()[0];
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.get_nickname() == new_nick && it->first != fd)
            return send_reply(fd, ":" + _hostname + " 433 * " + new_nick + " :Nickname is already in use");
    }

    if (_clients[fd].get_is_registered())
    {
        std::string old_prefix = ":" + _clients[fd].getPrefix();
        _clients[fd].set_nickname(new_nick);
        for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        {
            if (it->second.isMember(&_clients[fd]))
                it->second.broadcast(old_prefix + " NICK " + new_nick, NULL);
        }
        return ;
    }

    _clients[fd].set_nickname(new_nick);
    _clients[fd].set_is_nick(true);
    if (_clients[fd].try_register())
        send_reply(fd, rpl("001", fd) + " :Welcome to the Internet Relay Network " + new_nick);
}

void Server::cmd_user(int fd, Message &msg)
{
    if (!_clients[fd].get_is_pass())
        return send_reply(fd, ":" + _hostname + " 451 * :You have not registered (Send PASS first)");
    if (_clients[fd].get_is_registered())
        return send_reply(fd, ":" + _hostname + " 462 * :You may not reregister");
    if (msg.getParams().size() < 4)
        return send_reply(fd, ":" + _hostname + " 461 * USER :Not enough parameters");
    _clients[fd].set_username(msg.getParams()[0]);
    _clients[fd].set_realname(msg.getParams()[3]);
    _clients[fd].set_is_user(true);
    if (_clients[fd].try_register())
        send_reply(fd, rpl("001", fd) + " :Welcome to the Internet Relay Network " + _clients[fd].get_nickname());
}

void Server::cmd_join(int fd, Message &msg)
{
    std::string nick = _clients[fd].get_nickname();
    if (msg.getParams().empty())
        return send_reply(fd, rpl("461", fd) + " JOIN :Not enough parameters");

    std::string chan_name = msg.getParams()[0];
    if (chan_name.empty() || chan_name[0] != '#')
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");

    bool is_new = (_channels.find(chan_name) == _channels.end());
    if (is_new)
        _channels[chan_name] = Channel(chan_name);
    Channel &chan = _channels[chan_name];

    if (chan.isMember(&_clients[fd]))
        return ;
    if (chan.isInviteOnly() && !chan.isInvited(nick))
        return send_reply(fd, rpl("473", fd) + " " + chan_name + " :Cannot join channel (+i)");
    if (!chan.getKey().empty())
    {
        std::string given = (msg.getParams().size() > 1) ? msg.getParams()[1] : "";
        if (given != chan.getKey())
            return send_reply(fd, rpl("475", fd) + " " + chan_name + " :Cannot join channel (+k)");
    }
    if (chan.getUserLimit() > 0 && chan.getMemberCount() >= chan.getUserLimit())
        return send_reply(fd, rpl("471", fd) + " " + chan_name + " :Cannot join channel (+l)");

    chan.addClient(&_clients[fd]);
    chan.removeInvite(nick);
    if (is_new)
        chan.giveOperator(&_clients[fd]);

    chan.broadcast(":" + _clients[fd].getPrefix() + " JOIN " + chan_name, NULL);
    if (!chan.getTopic().empty())
        send_reply(fd, rpl("332", fd) + " " + chan_name + " :" + chan.getTopic());
    send_reply(fd, rpl("353", fd) + " = " + chan_name + " :" + build_names_list(chan));
    send_reply(fd, rpl("366", fd) + " " + chan_name + " :End of /NAMES list");
}

void Server::cmd_part(int fd, Message &msg)
{
    if (msg.getParams().empty())
        return send_reply(fd, rpl("461", fd) + " PART :Not enough parameters");
    std::string chan_name = msg.getParams()[0];
    if (_channels.find(chan_name) == _channels.end())
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");
    Channel &chan = _channels[chan_name];
    if (!chan.isMember(&_clients[fd]))
        return send_reply(fd, rpl("442", fd) + " " + chan_name + " :You're not on that channel");

    std::string reason = (msg.getParams().size() > 1) ? msg.getParams()[1] : "Leaving";
    chan.broadcast(":" + _clients[fd].getPrefix() + " PART " + chan_name + " :" + reason, NULL);
    chan.removeClient(&_clients[fd]);
    if (chan.getMemberCount() == 0)
        _channels.erase(chan_name);
}

void Server::cmd_privmsg(int fd, Message &msg)
{
    if (msg.getParams().empty())
        return send_reply(fd, rpl("411", fd) + " :No recipient given (PRIVMSG)");
    if (msg.getParams().size() < 2)
        return send_reply(fd, rpl("412", fd) + " :No text to send");

    std::string target = msg.getParams()[0];
    std::string text = msg.getParams()[1];
    std::string prefix = ":" + _clients[fd].getPrefix();

    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
            return send_reply(fd, rpl("403", fd) + " " + target + " :No such channel");
        Channel &chan = _channels[target];
        if (!chan.isMember(&_clients[fd]))
            return send_reply(fd, rpl("404", fd) + " " + target + " :Cannot send to channel");
        chan.broadcast(prefix + " PRIVMSG " + target + " :" + text, &_clients[fd]);
    }
    else
    {
        Client *dest = find_client_by_nick(target);
        if (!dest)
            return send_reply(fd, rpl("401", fd) + " " + target + " :No such nick/channel");
        send_reply(dest->get_fd(), prefix + " PRIVMSG " + target + " :" + text);
    }
}

void Server::cmd_quit(int fd, Message &msg, size_t &index)
{
    std::string reason = msg.getParams().empty() ? "Client quit" : msg.getParams()[0];
    std::string prefix = ":" + _clients[fd].getPrefix();

    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end();)
    {
        if (it->second.isMember(&_clients[fd]))
        {
            it->second.broadcast(prefix + " QUIT :" + reason, &_clients[fd]);
            it->second.removeClient(&_clients[fd]);
        }
        if (it->second.getMemberCount() == 0)
            _channels.erase(it++);
        else
            ++it;
    }
    close(fd);
    _clients.erase(fd);
    _pollfds.erase(_pollfds.begin() + index);
    index--;
}

void Server::cmd_kick(int fd, Message &msg)
{
    if (msg.getParams().size() < 2)
        return send_reply(fd, rpl("461", fd) + " KICK :Not enough parameters");
    std::string chan_name = msg.getParams()[0];
    std::string target_nick = msg.getParams()[1];
    std::string reason = (msg.getParams().size() > 2) ? msg.getParams()[2] : _clients[fd].get_nickname();

    if (_channels.find(chan_name) == _channels.end())
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");
    Channel &chan = _channels[chan_name];
    if (!chan.isMember(&_clients[fd]))
        return send_reply(fd, rpl("442", fd) + " " + chan_name + " :You're not on that channel");
    if (!chan.isOperator(&_clients[fd]))
        return send_reply(fd, rpl("482", fd) + " " + chan_name + " :You're not channel operator");
    Client *target = find_client_by_nick(target_nick);
    if (!target || !chan.isMember(target))
        return send_reply(fd, rpl("441", fd) + " " + target_nick + " " + chan_name + " :They aren't on that channel");

    chan.broadcast(":" + _clients[fd].getPrefix() + " KICK " + chan_name + " " + target_nick + " :" + reason, NULL);
    chan.removeClient(target);
    if (chan.getMemberCount() == 0)
        _channels.erase(chan_name);
}

void Server::cmd_invite(int fd, Message &msg)
{
    if (msg.getParams().size() < 2)
        return send_reply(fd, rpl("461", fd) + " INVITE :Not enough parameters");
    std::string target_nick = msg.getParams()[0];
    std::string chan_name = msg.getParams()[1];

    if (_channels.find(chan_name) == _channels.end())
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");
    Channel &chan = _channels[chan_name];
    if (!chan.isMember(&_clients[fd]))
        return send_reply(fd, rpl("442", fd) + " " + chan_name + " :You're not on that channel");
    if (chan.isInviteOnly() && !chan.isOperator(&_clients[fd]))
        return send_reply(fd, rpl("482", fd) + " " + chan_name + " :You're not channel operator");
    Client *target = find_client_by_nick(target_nick);
    if (!target)
        return send_reply(fd, rpl("401", fd) + " " + target_nick + " :No such nick/channel");
    if (chan.isMember(target))
        return send_reply(fd, rpl("443", fd) + " " + target_nick + " " + chan_name + " :is already on channel");

    chan.addInvite(target_nick);
    send_reply(fd, rpl("341", fd) + " " + target_nick + " " + chan_name);
    send_reply(target->get_fd(), ":" + _clients[fd].getPrefix() + " INVITE " + target_nick + " " + chan_name);
}

void Server::cmd_topic(int fd, Message &msg)
{
    if (msg.getParams().empty())
        return send_reply(fd, rpl("461", fd) + " TOPIC :Not enough parameters");
    std::string chan_name = msg.getParams()[0];
    if (_channels.find(chan_name) == _channels.end())
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");
    Channel &chan = _channels[chan_name];
    if (!chan.isMember(&_clients[fd]))
        return send_reply(fd, rpl("442", fd) + " " + chan_name + " :You're not on that channel");

    if (msg.getParams().size() < 2)
    {
        if (chan.getTopic().empty())
            send_reply(fd, rpl("331", fd) + " " + chan_name + " :No topic is set");
        else
            send_reply(fd, rpl("332", fd) + " " + chan_name + " :" + chan.getTopic());
        return ;
    }
    if (chan.isTopicRestricted() && !chan.isOperator(&_clients[fd]))
        return send_reply(fd, rpl("482", fd) + " " + chan_name + " :You're not channel operator");
    chan.setTopic(msg.getParams()[1]);
    chan.broadcast(":" + _clients[fd].getPrefix() + " TOPIC " + chan_name + " :" + msg.getParams()[1], NULL);
}

void Server::cmd_mode(int fd, Message &msg)
{
    if (msg.getParams().empty())
        return send_reply(fd, rpl("461", fd) + " MODE :Not enough parameters");
    std::string chan_name = msg.getParams()[0];
    if (chan_name[0] != '#')
        return ;
    if (_channels.find(chan_name) == _channels.end())
        return send_reply(fd, rpl("403", fd) + " " + chan_name + " :No such channel");
    Channel &chan = _channels[chan_name];

    if (msg.getParams().size() < 2)
    {
        std::string modes = "+";
        std::string mparams;
        if (chan.isInviteOnly()) modes += "i";
        if (chan.isTopicRestricted()) modes += "t";
        if (!chan.getKey().empty()) { modes += "k"; mparams += " " + chan.getKey(); }
        if (chan.getUserLimit() > 0)
        {
            modes += "l";
            std::ostringstream oss;
            oss << chan.getUserLimit();
            mparams += " " + oss.str();
        }
        return send_reply(fd, rpl("324", fd) + " " + chan_name + " " + modes + mparams);
    }

    if (!chan.isOperator(&_clients[fd]))
        return send_reply(fd, rpl("482", fd) + " " + chan_name + " :You're not channel operator");

    std::string mode_str = msg.getParams()[1];
    bool adding = true;
    size_t pi = 2;
    std::string amodes, aparams;
    bool sign_set = false;
    bool last_plus = true;

    for (size_t i = 0; i < mode_str.size(); i++)
    {
        char c = mode_str[i];
        if (c == '+') { adding = true; continue ; }
        if (c == '-') { adding = false; continue ; }

        if (c == 'i')
        {
            chan.setInviteOnly(adding);
        }
        else if (c == 't')
        {
            chan.setTopicRestricted(adding);
        }
        else if (c == 'k')
        {
            if (adding)
            {
                if (pi >= msg.getParams().size())
                    { send_reply(fd, rpl("461", fd) + " MODE :Not enough parameters"); continue ; }
                std::string key = msg.getParams()[pi++];
                chan.setKey(key);
                if (!sign_set || last_plus != adding) { amodes += "+"; last_plus = true; sign_set = true; }
                amodes += "k";
                aparams += " " + key;
                continue ;
            }
            chan.setKey("");
        }
        else if (c == 'o')
        {
            if (pi >= msg.getParams().size())
                { send_reply(fd, rpl("461", fd) + " MODE :Not enough parameters"); continue ; }
            std::string tnick = msg.getParams()[pi++];
            Client *t = find_client_by_nick(tnick);
            if (!t || !chan.isMember(t))
                { send_reply(fd, rpl("441", fd) + " " + tnick + " " + chan_name + " :They aren't on that channel"); continue ; }
            if (adding) chan.giveOperator(t);
            else chan.removeOperator(t);
            if (!sign_set || last_plus != adding) { amodes += adding ? "+" : "-"; last_plus = adding; sign_set = true; }
            amodes += "o";
            aparams += " " + tnick;
            continue ;
        }
        else if (c == 'l')
        {
            if (adding)
            {
                if (pi >= msg.getParams().size())
                    { send_reply(fd, rpl("461", fd) + " MODE :Not enough parameters"); continue ; }
                int limit = std::atoi(msg.getParams()[pi++].c_str());
                if (limit <= 0)
                    { send_reply(fd, rpl("461", fd) + " MODE :Invalid limit"); continue ; }
                chan.setUserLimit(limit);
                if (!sign_set || last_plus != adding) { amodes += "+"; last_plus = true; sign_set = true; }
                amodes += "l";
                std::ostringstream oss; oss << limit;
                aparams += " " + oss.str();
                continue ;
            }
            chan.setUserLimit(0);
        }
        else
        {
            send_reply(fd, rpl("472", fd) + " " + std::string(1, c) + " :is unknown mode char to me");
            continue ;
        }
        if (!sign_set || last_plus != adding) { amodes += adding ? "+" : "-"; last_plus = adding; sign_set = true; }
        amodes += c;
    }
    if (!amodes.empty())
        chan.broadcast(":" + _clients[fd].getPrefix() + " MODE " + chan_name + " " + amodes + aparams, NULL);
}

void Server::cmd_ping(int fd, Message &msg)
{
    std::string token = msg.getParams().empty() ? "" : msg.getParams()[0];
    send_reply(fd, ":" + _hostname + " PONG " + _hostname + " :" + token);
}

void Server::dispatch_cmd(int fd, Message &msg, size_t &index)
{
    std::string cmd = msg.getCmd();
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = std::toupper(cmd[i]);

    if (cmd == "PASS")        cmd_pass(fd, msg);
    else if (cmd == "NICK")   cmd_nick(fd, msg);
    else if (cmd == "USER")   cmd_user(fd, msg);
    else if (cmd == "PING")   cmd_ping(fd, msg);
    else if (!_clients[fd].get_is_registered())
        send_reply(fd, ":" + _hostname + " 451 * :You have not registered");
    else if (cmd == "JOIN")   cmd_join(fd, msg);
    else if (cmd == "PART")   cmd_part(fd, msg);
    else if (cmd == "PRIVMSG") cmd_privmsg(fd, msg);
    else if (cmd == "QUIT")   cmd_quit(fd, msg, index);
    else if (cmd == "KICK")   cmd_kick(fd, msg);
    else if (cmd == "INVITE") cmd_invite(fd, msg);
    else if (cmd == "TOPIC")  cmd_topic(fd, msg);
    else if (cmd == "MODE")   cmd_mode(fd, msg);
    else
        send_reply(fd, rpl("421", fd) + " " + cmd + " :Unknown command");
}

void Server::add_new_client()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(_srv_socket, (struct sockaddr*)&client_addr, &client_len);
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
    _pollfds.push_back(pfd);
    _clients[client_fd] = Client(client_fd, ip);
}

void Server::parse_client_message(size_t &index)
{
    char buffer[1024];
    Message msg;
    int fd = _pollfds[index].fd;
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0)
    {
        std::cout << "Client " << fd << " disconnected." << std::endl;
        remove_client(fd);
        close(fd);
        _clients.erase(fd);
        _pollfds.erase(_pollfds.begin() + index);
        index--;
        return ;
    }

    buffer[bytes] = '\0';
    _clients[fd].push_back_buf(buffer);
    std::string &buf = _clients[fd].get_recv_buf();
    size_t pos;

    while ((pos = buf.find('\n')) != std::string::npos)
    {
        std::string line = buf.substr(0, pos);
        buf.erase(0, pos + 1);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (!line.empty())
        {
            msg.parse(line);
            dispatch_cmd(fd, msg, index);
        }
    }
}

void Server::run()
{
    std::cout << "Starting server..." << std::endl;
    _srv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_srv_socket == -1)
    {
        std::cerr << "Error: could not create socket" << std::endl;
        return ;
    }
    int opt = 1;
    if (setsockopt(_srv_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Error: could not set socket options" << std::endl;
        close(_srv_socket);
        return ;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(_port);
    serv.sin_addr.s_addr = INADDR_ANY;

    if (bind(_srv_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0)
    {
        std::cerr << "Error: binding socket" << std::endl;
        close(_srv_socket);
        return ;
    }
    if (listen(_srv_socket, SOMAXCONN) < 0)
    {
        std::cerr << "Error: listening on socket" << std::endl;
        close(_srv_socket);
        return ;
    }

    std::cout << "IRC Server is online on port " << _port << std::endl;

    struct pollfd pfd;
    pfd.fd = _srv_socket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    while (g_is_running)
    {
        if (poll(&_pollfds[0], _pollfds.size(), -1) < 0)
        {
            if (!g_is_running)
                break ;
            std::cerr << "Error: poll() failed" << std::endl;
            break ;
        }
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                if (_pollfds[i].fd == _srv_socket)
                    add_new_client();
                else
                    parse_client_message(i);
            }
        }
    }
}

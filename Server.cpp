#include "Server.hpp"

Server::Server()
{

}

Server::Server(const Server &other) : password(other.password), port(other.port)
{

}

Server::Server(std::string passwd, int prt) : password(passwd), port(prt)
{

}

Server::~Server()
{

}

Server& Server::operator=(const Server &obj)
{
    if(this != &obj)
    {
        this->password = obj.password;
        this->port = obj.port;
        this->srv_socket = obj.srv_socket;
    }
    return *this;
}

/**
 * send_reply()
 * helper function to send raw irc numeric replies to clients.
 * it automatically appends the \r\n that irc rfc demands.
 * right now we directly call send(), but later on we might need...
 * to push this into a client send buffer to handle non-blocking socket issues.
 * also, the ":server" prefix is hardcoded and needs to be changed later to our actual server name.
 */
void Server::send_reply(int fd, const std::string &reply)
{
	std::string full_reply = reply + "\r\n";
	send(fd, full_reply.c_str(), full_reply.length(), 0);
}

/**
 * cmd_pass()
 * handles the PASS command which must be sent before NICK or USER.
 * simply checks if the password matches our server password and sets the boolean.
 * note for later: right now if they give a wrong password we just send ERR 464...
 * we might want to immediately close(fd) and drop the connection when authentication fails.
 */
void Server::cmd_pass(int fd, Message &msg)
{
	if (client[fd].get_is_registered())
	{
		send_reply(fd, ":server 462 * :You may not reregister");
		return ;
	}
	if (msg.getParams().empty())
	{
		send_reply(fd, ":server 461 * PASS :Not enough parameters");
		return ;
	}
	if (msg.getParams()[0] == this->password)
	{
		client[fd].set_is_pass(true);
	}
	else
	{
		send_reply(fd, ":server 464 * :Password incorrect");
	}
}
/**
 * cmd_nick()
 * handles setting the nickname. first checks if PASS was already sent.
 * loops through the whole client map to make sure the nickname isn't already taken.
 * if they already set their USER too, it triggers try_register() and sends the welcome message.
 * todo for later: we need to add character validation so users can't use special symbols...
 * and handle nickname changes if the client is already registered.
 */
void Server::cmd_nick(int fd, Message &msg)
{
	if (!client[fd].get_is_pass())
	{
		send_reply(fd, ":server 451 * :You have not registered (Send PASS first)");
		return ;
	}
	if (msg.getParams().empty())
	{
		send_reply(fd, ":server 431 * :No nickname given");
		return ;
	}

	std::string new_nick = msg.getParams()[0];

	for (std::map<int, Client>::iterator it = client.begin(); it != client.end(); ++it)
	{
		if (it->second.get_nickname() == new_nick && it->first != fd)
		{
			send_reply(fd, ":server 433 * " + new_nick + " :Nickname is already in use");
			return ;
		}
	}
	client[fd].set_nickname(new_nick);
	client[fd].set_is_nick(true);
	if (client[fd].try_register())
	{
		send_reply(fd, ":server 001 " + new_nick + " :Welcome to the Internet Relay Network " + new_nick);
	}
}


/**
 * cmd_user()
 * handles the USER command to set username and realname (params 0 and 3).
 * ignores hostname and servername since modern irc servers just use the ip anyway.
 * if NICK was already set, this completes registration and sends the 001 welcome rpl.
 * just make sure our Message::parse() correctly grabs the whole realname if it has spaces!
 */
void Server::cmd_user(int fd, Message &msg)
{
	if (!client[fd].get_is_pass())
	{
		send_reply(fd, ":server 451 * :You have not registered (Send PASS first)");
		return ;
	}
	if (client[fd].get_is_registered())
	{
		send_reply(fd, ":server 462 * :You may not reregister");
		return ;
	}
	if (msg.getParams().size() < 4)
	{
		send_reply(fd, ":server 461 * USER :Not enough parameters");
		return ;
	}
	client[fd].set_username(msg.getParams()[0]);
	client[fd].set_realname(msg.getParams()[3]);
	client[fd].set_is_user(true);
	if (client[fd].try_register())
	{
		send_reply(fd, ":server 001 " + client[fd].get_nickname() + " :Welcome to the Internet Relay Network " + client[fd].get_nickname());
	}
}
/**
 * dispatch_cmd()
 * is command dispatcher, has a huge chain of hardcoded if/else
 * later on it needs to get cleaned to something else other than if/else
 * cannot use switch case on std::string on c++98
 * this method has only one purpose, in which it routes commands...
 * to their respective executers which I have yet to declare, no more no less.
 */

void Server::dispatch_cmd(int fd, Message &msg)
{
	std::string cmd = msg.getCmd();

	if (cmd == "PASS")
	{
		cmd_pass(fd, msg);
	}
	else if (cmd == "NICK")
	{
		cmd_nick(fd, msg);
	}
	else if (cmd == "USER")
	{
		cmd_user(fd, msg);
	}
	else if (!client[fd].get_is_registered())
	{
		send_reply(fd, ":server 451 * :You have not registered");
	}
	else
	{
		// Future registered commands go here (e.g., JOIN, PRIVMSG, KICK, etc.)
	}
}
/** 
 * add_new_client()
 * this method's sole purpose is to accept tcp connection and only that.
 * if the user connects with the password through an irc client...
 * the parse method in the Message class should be ready to accept...
 * 3 consecutive commands, which are the PASS, NICK and USER.
 * these 3 commands are received automatically by the server from the irc client.
 * add_new_client() method is complete and doesn't need to altered in any way.
*/

void Server::add_new_client()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = accept(srv_socket, (struct sockaddr*)&client_addr, &client_len);
    
    if (client_fd == -1) {
        std::cerr << "Error: could not accept user." << std::endl;
        return;
    }
    
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Error: fcntl failed." << std::endl;
        close(client_fd);
        return;
    }

    std::string ip_address = inet_ntoa(client_addr.sin_addr);
    std::cout << "Client connected Socket ID: " << client_fd << " IP: " << ip_address << std::endl;
    
    struct pollfd new_client;
    new_client.fd = client_fd;
    new_client.events = POLLIN;
    new_client.revents = 0;
    _pollfds.push_back(new_client);

    Client new_user(client_fd, ip_address); 
    client[client_fd] = new_user;
}

void Server::parse_client_message(size_t &index)
{
    char buffer[1024];
	Message msg;
	int fd = _pollfds[index].fd;
    int bytes_received = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received <= 0) {
		// remove the client from any channel they were in
		// notify said channel that the client disconnected
		// need to create a method that takes care of the below code.
        std::cout << "Client " << fd << " disconnected." << std::endl;
        close(fd);
		client.erase(fd);
        _pollfds.erase(_pollfds.begin() + index);
        index--;
		return;
    }

	buffer[bytes_received] = '\0';
	client[fd].push_back_buf(buffer);
	size_t pos = client[fd].get_recv_buf().find("\r\n");
	while (pos != std::string::npos) {
		std::string line = client[fd].get_recv_buf().substr(0, pos);
		msg.parse(line);
		dispatch_cmd(fd, msg);
		client[fd].get_recv_buf().erase(0, pos + 2);
		pos = client[fd].get_recv_buf().find("\r\n");
	}
}

void Server::run()
{
    struct sockaddr_in serv;

    std::cout << "Starting server...\n";

    srv_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (srv_socket == -1) {
        std::cerr << "Error: could not create socket" << std::endl;
        return;
    }

	int opt = 1;
	if (setsockopt(srv_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "Error: could not set socket options" << std::endl;
		close(srv_socket);
		return;
	}

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
	serv.sin_addr.s_addr = INADDR_ANY;

    if(bind(srv_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return;
    }

    if(listen(srv_socket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return;
    }

    std::cout << "IRC Server is online.\n";
    std::cout << "Listening on port " << port << std::endl;

    struct pollfd pfd;
    pfd.fd = srv_socket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);

    while (true)
    {
        if (poll(&_pollfds[0], _pollfds.size(), -1) < 0) {
            // handle error
        }
        else {
            for (size_t i = 0; i < _pollfds.size(); i++)
            {
                if (_pollfds[i].revents & POLLIN)
                {
                    if (_pollfds[i].fd == srv_socket) {
                        add_new_client();
                    }
                    else {
                        parse_client_message(i);
                    }
                }
            }
        }
    }
}

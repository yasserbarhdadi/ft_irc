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

	if (cmd == "PASS") {

	}
	else if (cmd == "NICK") {

	}
	else if (cmd == "USER") {

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
    int bytes_received = recv(_pollfds[index].fd, buffer, sizeof(buffer) - 1, 0);
	int fd = _pollfds[index].fd;

    if (bytes_received <= 0) {
		// missing client cleanup
		// need to remove the client from the client map
		// remove the client from any channel they were in
		// notify said channel that the client disconnected
        std::cout << "Client disconnected: " << _pollfds[index].fd << std::endl;
        close(_pollfds[index].fd);
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

//pass permision accept

bool Server::authenticate_client(Client &client, const std::string &input_password)
{
	if (input_password != this->password)
	{
		return (false);
	}
	client.set_is_registered(true);
	return (true);
}
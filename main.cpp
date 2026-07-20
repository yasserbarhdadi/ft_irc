#include "Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3) {
        std::cerr << "Usage: /ircserv <port> <password>" << std::endl;
        return 1;
    }
    Server server(av[2], (int)(std::strtol(av[1], NULL, 10)));
    server.run();
    return 0;
}

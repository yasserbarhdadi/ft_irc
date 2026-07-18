#include "main.hpp"

int main(int ac, char **av)
{
    if (ac != 3) {
        std::cerr << "Usage: /ircserv <port> <password>" << std::endl;
        return 1;
    }
    
    Server server(av[2], atoi(av[1]));
    server.run();
    return 0;
}

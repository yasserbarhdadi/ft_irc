#include "Server.hpp"

bool g_is_running = true;

void signalHandler(int signum)
{
    (void)signum;
    std::cout << "\nShutting down..." << std::endl;
    g_is_running = false;
}

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    char *endptr;
    long port = std::strtol(av[1], &endptr, 10);
    if (*endptr != '\0' || port <= 0 || port > 65535)
    {
        std::cerr << "Error: invalid port number (must be 1-65535)" << std::endl;
        return 1;
    }
    if (std::string(av[2]).empty())
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }
    signal(SIGINT, signalHandler);
    Server server(av[2], static_cast<int>(port));
    server.run();
    return 0;
}

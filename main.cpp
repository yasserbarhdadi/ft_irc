#include "Server.hpp"

bool g_is_running = true;

void signalHandler(int signum)
{
    (void)signum;
    std::cout << "\n Ctrl+C detected. Shutting down ..." << std::endl;
    g_is_running = false; 
}


int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: /ircserv <port> <password>" << std::endl;
        return 1;
    }
    signal(SIGINT, signalHandler);
    Server server(av[2], (int)(std::strtol(av[1], NULL, 10)));
    server.run();
    return 0;
    //test hada
}

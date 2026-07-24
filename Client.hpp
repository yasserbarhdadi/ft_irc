#pragma once
#include "main.hpp"

class Client
{
    private:
        int         fd;
        std::string ip_address;
        std::string nickname;
        std::string username;
        std::string realname;
        std::string recv_buffer;
        bool        is_registered;
        bool        is_pass;
        bool        is_nick;
        bool        is_user;

    public:
        Client();
        Client(int fd);
        Client(int fd, std::string ip_addr);
        ~Client();
        Client(const Client &other);
        Client &operator=(const Client &other);

        int         get_fd() const;
        std::string getPrefix() const;

        bool        get_is_registered() const;
        void        set_is_registered(bool value);
        void        push_back_buf(char *buf);
        std::string &get_recv_buf();

        bool        get_is_pass() const;
        void        set_is_pass(bool value);
        bool        get_is_nick() const;
        void        set_is_nick(bool value);
        bool        get_is_user() const;
        void        set_is_user(bool value);
        bool        try_register();

        std::string get_nickname() const;
        void        set_nickname(const std::string &nick);
        std::string get_username() const;
        void        set_username(const std::string &user);
        std::string get_realname() const;
        void        set_realname(const std::string &real);
};
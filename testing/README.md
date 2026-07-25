*This project has been created as part of the 42 curriculum by yabarhda, yel-joul, zakel-kh.*

# ft_irc

## Description

**ft_irc** is a custom implementation of an IRC (Internet Relay Chat) server, written from scratch in C++98.

IRC is a text-based protocol that lets people connect to a server and exchange messages in real time, either directly with one another or in shared channels. The goal of this project is to understand and reproduce the core mechanics of that protocol: accepting multiple client connections at once, parsing the commands they send, and dispatching the right behavior for each one, all without ever blocking or crashing.

The server is tested against a real, unmodified IRC client (not a client we wrote ourselves), and supports:

- Client registration via `PASS`, `NICK`, and `USER`
- Joining and leaving channels (`JOIN`, `PART`)
- Sending private messages to a user or a channel (`PRIVMSG`)
- A distinction between regular users and channel operators
- Operator-only channel commands:
  - `KICK` — eject a user from a channel
  - `INVITE` — invite a user to a channel
  - `TOPIC` — view or change a channel's topic
  - `MODE` — change channel modes (`i` invite-only, `t` topic restriction, `k` key/password, `o` operator privilege, `l` user limit)

Everything runs through a single `poll()` loop watching all client sockets at once — no forking, no blocking reads or writes. Data is read as it arrives and reassembled until a full line is available, and outgoing messages are queued and only flushed once the socket reports it's ready to be written to.

## Instructions

### Compilation

```bash
make
```

This builds the `ircserv` executable using `c++` with the flags `-Wall -Wextra -Werror -std=c++98`.

Other available Makefile rules:

```bash
make clean   # remove object files
make fclean  # remove object files and the executable
make re      # fclean + all
```

### Execution

```bash
./ircserv <port> <password>
```

- `port` — the port the server will listen on <your_port>
- `password` — the password clients must provide to connect

Example:

```bash
./ircserv 8080 mypassword
```

### Connecting

Connect using any standard IRC client (e.g. `irssi`, `WeeChat`, `HexChat`), or manually with `nc` for quick testing:

```bash
nc 127.0.0.1 8080
PASS mypassword
NICK laarbi
USER laarbi 0 * :Laarbi
JOIN #general
```

## Command Reference

### Connection & Registration

```
PASS <password>              →  PASS mypass123
NICK <nickname>              →  NICK john
USER <user> 0 * :<realname>  →  USER john 0 * :John Doe
```

These three must be sent in order: `PASS` first, then `NICK` and `USER` (in any order).

### Channel Commands

```
JOIN #<channel> [key]        →  JOIN #general
                              →  JOIN #secret mykey
PART #<channel> [:<reason>]  →  PART #general
                              →  PART #general :bye everyone
TOPIC #<channel>             →  TOPIC #general              (view topic)
TOPIC #<channel> :<topic>    →  TOPIC #general :Welcome!    (set topic)
```

### Messaging

```
PRIVMSG #<channel> :<text>   →  PRIVMSG #general :hello everyone
PRIVMSG <nickname> :<text>   →  PRIVMSG john :hey what's up
```

### Channel Operator Commands (requires @)

```
KICK #<channel> <nick> [:<reason>]  →  KICK #general troll :behave
INVITE <nick> #<channel>            →  INVITE john #vip
MODE #<channel> +i                  →  set invite-only
MODE #<channel> -i                  →  remove invite-only
MODE #<channel> +t                  →  only ops can change topic
MODE #<channel> -t                  →  anyone can change topic
MODE #<channel> +k <key>            →  MODE #general +k secret123
MODE #<channel> -k                  →  remove channel password
MODE #<channel> +o <nick>           →  MODE #general +o john
MODE #<channel> -o <nick>           →  MODE #general -o john
MODE #<channel> +l <limit>          →  MODE #general +l 10
MODE #<channel> -l                  →  remove user limit
MODE #<channel> +itk secret         →  combine multiple modes
```

## Technical Choices

- **I/O model**: a single `poll()` call multiplexes the listening socket and every connected client socket. All sockets are non-blocking.
- **Output buffering**: outgoing data is never written directly with `send()`. It is appended to a per-client buffer and flushed only when `poll()` reports the socket as writable (`POLLOUT`), so a slow or unresponsive client can never stall the rest of the server.
- **Input buffering**: incoming bytes are appended to a per-client buffer and split on `\r\n` / `\n`, so a command split across several `recv()` calls (e.g. sent character by character) is still parsed correctly.
- **Code organization**: command handlers are grouped by purpose into separate `.cpp` files rather than piled into `Server.cpp`:
  - `Server.cpp` — connection lifecycle, the poll loop, and command dispatch
  - `CommandsRegistration.cpp` — `PASS`, `NICK`, `USER`, `PING`
  - `CommandsChannel.cpp` — `JOIN`, `PART`, `PRIVMSG`
  - `CommandsOperator.cpp` — `KICK`, `INVITE`, `TOPIC`, `MODE`
  - `Channel.cpp` / `Client.cpp` / `Message.cpp` — the data model and raw-line parser

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- `man poll`, `man fcntl`, `man socket`, `man setsockopt`
- [modernIRC — a friendlier, annotated reference for the protocol](https://modern.ircdocs.horse/)

### Use of AI

No AI-generated code was used to implement the core IRC protocol logic itself (parsing, registration, channel handling, command semantics); that logic was written by the team and only reviewed with AI assistance.
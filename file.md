# ft_irc — Modification Log

This document details every modification made to the project.

---

## 1. Client.hpp

- Removed unused 5-argument constructor
- Added `get_fd()` — returns the client's socket fd, needed by broadcast and direct messaging
- Added `getPrefix()` — builds `nick!user@host` IRC prefix string for protocol-compliant messages
- Removed all section comments, consistent alignment

## 2. Client.cpp

- **Fixed undefined behavior**: all constructors now initialize `is_pass`, `is_nick`, `is_user` to `false` (previously uninitialized)
- **Fixed copy constructor**: now copies `recv_buffer`, `is_pass`, `is_nick`, `is_user` (previously missing)
- **Fixed operator=**: same missing fields now copied
- Removed redundant `this->` throughout
- Compacted one-liner getters/setters onto single lines
- Removed unused 5-arg constructor
- Added `get_fd()` and `getPrefix()` implementations

## 3. Channel.hpp

- Added `_name` field — channel didn't know its own name
- Added `_topic` field — for TOPIC command
- Added `_key` field — channel password for MODE +k
- Added `_inviteOnly` flag — MODE +i
- Added `_topicRestricted` flag — MODE +t
- Added `_userLimit` field — MODE +l
- Added `_invited` list — invite list for +i mode
- Added `removeClient()` — clients could never leave
- Added `broadcast()` — send message to all members
- Added `getMembers()` — needed for RPL_NAMREPLY
- Added `getMemberCount()` — needed for +l limit check
- Added all mode getters/setters and invite list methods
- Renamed `members`/`operators` to `_members`/`_operators` for consistency

## 4. Channel.cpp

- Full implementation of all new methods
- `removeClient()` removes from both `_members` and `_operators` to prevent dangling pointers
- `broadcast()` sends to all members except one (the sender), uses `\r\n` line ending
- All invite list methods for invite-only channel management
- Compacted one-liner getters/setters

## 5. Message.hpp

- Removed unused `<map>` include

## 6. Message.cpp

- Compacted variable names (`space_index` → `sp`)
- Cleaner bracing style

## 7. Server.hpp

- Added `_hostname` member — replaces all hardcoded `":server"` strings
- Added `_channels` map — server had no way to store channels
- Added `rpl()` helper — builds `":hostname code nick"` prefix to avoid repetition
- Added all command handler declarations: `cmd_join`, `cmd_part`, `cmd_privmsg`, `cmd_quit`, `cmd_kick`, `cmd_invite`, `cmd_topic`, `cmd_mode`, `cmd_ping`
- Added helper methods: `remove_client`, `find_client_by_nick`, `build_names_list`
- Renamed `client` → `_clients`, `password` → `_password`, `port` → `_port`, `srv_socket` → `_srv_socket`
- `dispatch_cmd` signature updated with `size_t &index` for QUIT handling
- Added `SERVER_NAME` define

## 8. Server.cpp

### Fixes
- **`_srv_socket` initialized to -1** in all constructors (was uninitialized → potential close of random fd)
- **Destructor now closes all fds** — previously empty, leaked all file descriptors on shutdown
- **`bind()` and `listen()` failures now close `_srv_socket`** — previously leaked
- **`listen()` backlog changed from hardcoded 5 to `SOMAXCONN`** — lets the OS use the optimal backlog
- **Hardcoded `":server"` replaced with `_hostname`** — used throughout all 100+ reply strings
- **Line parser simplified** — was two-branch (`\r\n` then `\n`), now single `\n` scan with `\r` strip, handles both `nc` and IRC clients
- **`remove_client()` called on disconnect** — was a TODO, now properly cleans up channels

### New Commands
- `cmd_join` — creates/joins channels, enforces +i/+k/+l, first user gets op, sends TOPIC + NAMREPLY
- `cmd_part` — leaves channel, notifies members, deletes empty channels
- `cmd_privmsg` — routes to channels or users, validates membership/existence
- `cmd_quit` — notifies channels, cleans up, closes fd
- `cmd_kick` — operator-only, ejects target from channel
- `cmd_invite` — invites user, adds to invite list for +i channels
- `cmd_topic` — view/change topic, +t restricts to operators
- `cmd_mode` — handles +i, +t, +k, +o, +l with compound mode strings
- `cmd_ping` — responds with PONG to keep IRC clients connected

### New Helpers
- `rpl()` — builds `:hostname code nickname` prefix, eliminates 100+ hardcoded string concatenations
- `find_client_by_nick()` — resolves nickname to Client pointer
- `build_names_list()` — builds NAMREPLY with @ prefix for operators
- `remove_client()` — cleans up client from all channels on disconnect

### Modified
- `cmd_nick` — now handles nick changes for already-registered clients, notifies channels
- `dispatch_cmd` — uppercase conversion for case-insensitive commands, routes all new commands, sends ERR_UNKNOWNCOMMAND (421)
- `parse_client_message` — calls `remove_client()`, simplified line parser
- `add_new_client` — compacted, removed unnecessary temp variable

## 9. main.hpp

- Added `<sstream>` include centrally (was in Server.cpp)

## 10. main.cpp

- Added port validation (1-65535, non-numeric rejection)
- Added empty password check
- Used `static_cast<int>` instead of C-style cast

## 11. Makefile

- No changes needed — already correct

---

## Summary

| Category | Count |
|---|---|
| Files modified | 9 |
| New IRC commands | 9 (JOIN, PART, PRIVMSG, QUIT, KICK, INVITE, TOPIC, MODE, PING) |
| Bugs fixed | 4 (uninitialized bools, incomplete copy, fd leaks, missing cleanup) |
| Hardcoded values removed | 2 (":server" prefix, listen backlog) |
| Comments | All removed |

---

## Command Usage Guide

### Connection & Registration

```
PASS <password>              →  PASS mypass123
NICK <nickname>              →  NICK john
USER <user> 0 * :<realname>  →  USER john 0 * :John Doe
```

These three must be sent in order: PASS first, then NICK and USER (in any order).

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

### Other

```
QUIT [:<reason>]             →  QUIT :see you later
PING <token>                 →  PING test123     (server replies PONG)
```

### Testing with nc

```bash
nc -C 127.0.0.1 6667
PASS mypass
NICK testuser
USER testuser 0 * :Test User
JOIN #test
PRIVMSG #test :hello from nc
QUIT :done
```

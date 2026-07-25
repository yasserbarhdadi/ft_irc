# ft_irc — Command Usage Guide

A quick reference for every command implemented by `ircserv`, in the order a
client will typically need them.

---

## 1. Connection & Registration

Every client must complete this sequence before anything else works. `PASS`
must come first; `NICK` and `USER` can follow in either order, but both are
required before the server sends the `001` welcome message.

```
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
```

**Example:**
```
PASS mypass123
NICK john
USER john 0 * :John Doe
```

| Field | Notes |
|---|---|
| `<password>` | Must match the password the server was launched with (`./ircserv <port> <password>`). Wrong password → `464` and the connection is closed. |
| `<nickname>` | 1–9 characters, must start with a letter, no spaces/commas. Duplicate nicknames are rejected with `433`. |
| `<username>` | Free-form, no spaces. |
| `0 * ` | Historical USER placeholders — always send them as shown; the server ignores their values. |
| `<realname>` | Anything after `:` — may contain spaces. |

**Common errors while registering:**
| Code | Meaning |
|---|---|
| `431` | No nickname given |
| `432` | Erroneous nickname |
| `433` | Nickname already in use |
| `451` | You have not registered (sent NICK/USER before PASS) |
| `461` | Not enough parameters |
| `462` | You may not reregister |
| `464` | Password incorrect |

---

## 2. Channel Commands

```
JOIN #<channel> [key]
PART #<channel> [:<reason>]
TOPIC #<channel>              (view topic)
TOPIC #<channel> :<topic>     (set topic)
```

**Examples:**
```
JOIN #general
JOIN #secret mykey
PART #general
PART #general :bye everyone
TOPIC #general
TOPIC #general :Welcome!
```

Notes:
- Channel names must start with `#`.
- The first person to `JOIN` a channel automatically becomes its operator.
- `[key]` is only required if the channel has mode `+k` set.
- Setting the topic requires operator rights only if the channel has mode
  `+t` set; otherwise any member can change it.

---

## 3. Messaging

```
PRIVMSG #<channel> :<text>
PRIVMSG <nickname> :<text>
```

**Examples:**
```
PRIVMSG #general :hello everyone
PRIVMSG john :hey what's up
```

Notes:
- You must be a member of a channel to message it (`404` otherwise).
- Messaging a nickname that isn't connected returns `401`.

---

## 4. Channel Operator Commands (require `@`)

```
KICK #<channel> <nick> [:<reason>]
INVITE <nick> #<channel>
MODE #<channel> +i          set invite-only
MODE #<channel> -i          remove invite-only
MODE #<channel> +t          only operators can change the topic
MODE #<channel> -t          anyone can change the topic
MODE #<channel> +k <key>    set a channel password
MODE #<channel> -k          remove the channel password
MODE #<channel> +o <nick>   grant operator status
MODE #<channel> -o <nick>   revoke operator status
MODE #<channel> +l <limit>  set a user limit
MODE #<channel> -l          remove the user limit
MODE #<channel>             view current channel modes
```

**Examples:**
```
KICK #general troll :behave
INVITE john #vip
MODE #general +k secret123
MODE #general +itk secret123
```

Notes:
- All of these except plain `MODE #<channel>` (viewing) require the caller
  to already be a channel operator (`482` otherwise).
- Multiple mode letters can be combined in one call: `MODE #chan +ol john`
  sets `+o` for `john` and then expects the next parameter to be the
  `+l` limit — parameters are consumed left to right in the same order as
  their mode letters. Keep letters and their parameters aligned:
  `MODE #chan +ok john secret123` → `+o john`, then `+k secret123`.
- `+i` with no free spot: joining without an invite fails with `473`.
- `+k` with a wrong/missing key: joining fails with `475`.
- `+l` reached: joining fails with `471`.

---

## 5. Leaving

```
QUIT [:<reason>]
```

**Example:**
```
QUIT :see you later
```

Disconnects the client from the server (and every channel it was in),
broadcasting a `QUIT` notice with the given reason to anyone who shared a
channel with them.

---

## 6. Keepalive

```
PING [<token>]
```

The server always replies with `PONG <token>`. Most clients send this
automatically to detect dead connections; you generally never need to type
it yourself.

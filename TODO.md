Here is the architectural breakdown of what belongs inside each class, why it belongs there, and how you should restrict access using public and private modifiers. This keeps your architecture strictly aligned with object-oriented principles.
### 1. The Server Class
This is the central nervous system of your program. It owns the network hardware and manages the lifecycle of everything else.
**What it should have:**
 * **Attributes:** The port number, the server password, the Master Socket file descriptor, the vector of monitor structures (pollfd), a map of connected Clients (linked by their file descriptors), and a map of active Channels (linked by their names).
 * **Methods:** Initialization (socket, bind, listen), the infinite monitor loop, a function to accept new clients, and a function to receive/send data.
**Why:** The Server acts as the ultimate authority. It holds the "global state" of your IRC world. Nothing happens without the server orchestrating it.
**Public vs. Private:**
 * **Private:** **All attributes.** No other class should ever be able to directly modify the server's password or mess with the monitor list. Internal helper functions (like accept_client or remove_client) should also be private.
 * **Public:** Only the absolute minimum needed to start the program. Usually, just a constructor and a single run() or start() method called from your main.cpp.
### 2. The Client Class
This represents a single human being (or script) connected to your server.
**What it should have:**
 * **Attributes:** Their specific file descriptor, their IP address, their Nickname, their Username, and their Realname. Crucially, it needs a boolean flag tracking if they are fully registered, and a personal string buffer to hold their incoming, half-finished network text.
 * **Methods:** Primarily Getters and Setters for the attributes, and a function to append new text to their personal buffer.
**Why:** Because TCP is messy and users are complex, you need an isolated container to hold a user's state. If a client sends half a message, this class remembers it so the Server doesn't have to.
**Public vs. Private:**
 * **Private:** **All attributes.** You do not want the Parser accidentally changing a user's nickname directly without running the proper validation checks first.
 * **Public:** The Getters (e.g., getNickname()) and Setters (e.g., setNickname()).
### 3. The Channel Class
This represents an isolated chat room.
**What it should have:**
 * **Attributes:** The channel name, the current topic, a list (or map) of standard members, a list of operators, and a list of invited users. It also needs variables to hold the five mandated MODE flags (is it invite-only? is there a password? what is the user limit?).
 * **Methods:** Functions to add a client, remove a client, check if a client is an operator, and a broadcast function that sends a specific message to every file descriptor currently inside the channel.
**Why:** Grouping clients together makes communication efficient. When someone sends a message to #general, the Server just asks the #general Channel object to run its broadcast method, rather than the Server manually looping through everyone to see who belongs where.
**Public vs. Private:**
 * **Private:** **All attributes.** The internal lists of members and flags must be protected.
 * **Public:** The methods to interact with the room (e.g., join(Client), leave(Client), changeTopic()).
### 4. The Parser Class
This is the translator. It takes raw network gibberish and turns it into organized commands.
**What it should have:**
 * **Attributes:** It actually doesn't need many, if any, attributes. It is generally a "stateless" class.
 * **Methods:** A primary extraction function that takes a raw string, splits off the Command word, handles the colon rule for trailing text, and organizes the Parameters into a list/vector. It also needs the routing logic (the switchboard) to trigger the correct server action.
**Why:** Single Responsibility Principle. If you put all the string-slicing logic and command-routing logic inside the Server class, your Server file will become a massive, unreadable 3,000-line monolith. The Parser keeps the dirty string-manipulation work isolated.
**Public vs. Private:**
 * **Private:** The gritty internal string-slicing tools (like trimWhitespace or splitBySpace).
 * **Public:** A single main parseCommand(string raw_text, Client sender) method that the Server calls whenever it pulls a complete line ending in \r\n from a client's buffer.
NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
SRC = main.cpp Server.cpp Client.cpp Message.cpp Channel.cpp registration_cmd.cpp operator_cmd.cpp channel_cmd.cpp
HEADER = main.hpp Server.hpp Client.hpp Message.hpp Channel.hpp
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = ircserv
SRC = main.cpp Client.cpp Message.cpp
HEADER = main.hpp Client.hpp Message.hpp
OBJ = $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
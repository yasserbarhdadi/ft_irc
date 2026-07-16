NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

SRCS		= Server.cpp \
			  Parser.cpp \
			  Message.cpp \
			  accept_new_client.cpp \
			  Client.cpp \
			  cmd_handler.cpp

OBJS		= $(SRCS:.cpp=.o)

HEADERS		= Server.hpp \
			  Message.hpp \
			  Client.hpp \
			  cmd_handler.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

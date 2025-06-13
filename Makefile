NAME        =   webserv
SRC         =   main.cpp config_parser.cpp helper_functions.cpp
OBJ         =   $(SRC:.cpp=.o)
CXX         =   c++
CXXFLAGS    =   -Wall -Wextra -Werror -std=c++98 -g

all:        $(NAME)

$(NAME):    $(OBJ) webserv.hpp config_parser.hpp
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

clean:
	rm -f $(OBJ)

fclean:     clean
	rm -f $(NAME)

re:         fclean all

.PHONY:     all clean fclean re
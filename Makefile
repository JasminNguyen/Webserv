NAME        = webserv
SRC         = main2.cpp config_parser.cpp helper_functions.cpp Exceptions.cpp \
				CGI.cpp CGISource.cpp Connection.cpp ListeningSocket.cpp \
				Request.cpp Response.cpp Socket.cpp StaticSource.cpp \
				Webserver.cpp
OBJ         = $(SRC:.cpp=.o)
DEP 		= $(SRC:.cpp=.d) # Dependency files for headers generated from source files (replace .cpp with .d)
DEPS 		= $(patsubst %.d, .deps/%.d, $(DEP)) # Adjust DEP filenames to be inside the .deps directory
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

$(NAME): $(OBJ) # Link object files into the final executable
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

# Compile .cpp files into .o files
# - Create .deps directory if it doesn't exist
# - Generate dependency files (.d) inside .deps
%.o: %.cpp
	@mkdir -p .deps
	$(CXX) $(CXXFLAGS) -MMD -MF .deps/$*.d -c $< -o $@

clean: # Remove object files and dependency directory
	rm -f $(OBJ)
	rm -rf .deps

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re

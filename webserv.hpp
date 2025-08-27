#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <stdint.h>
# include <sys/socket.h>
# include <stdio.h>
# include <netinet/in.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/errno.h>
# include <poll.h>
# include <vector>
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <map>
# include <sys/stat.h>
# include <dirent.h>
# include <time.h>

//helper functions
std::string trim(const std::string& str);


#endif

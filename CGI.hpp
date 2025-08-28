#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include "config_parser.hpp"

class Request;
class Webserver;
class Connection;


class CGI
{
    private:


    public:

    static void run_cgi(Request & request, configParser::ServerConfig & server_block, Webserver & webserver, Connection &conn);
    static std::string  construct_script_path(Request & request, configParser::ServerConfig & server_block);
    static char **construct_argv(const char* &script_path, Request &request);
    static char **construct_envp(Request& request, configParser::ServerConfig & server_block);

};

#endif

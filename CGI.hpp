#ifndef CGI_HPP
#define CGI_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "config_parser.hpp"

class CGI
{
    private:


    public:

    int run_cgi(Request & request, configParser::ServerConfig & server_block);
    std::string  construct_script_path(Request & request, configParser::ServerConfig & server_block);
    char **construct_argv(const char* &script_path);
    char **construct_envp(Request& request, configParser::ServerConfig & server_block);

};

#endif

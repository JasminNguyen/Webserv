#ifndef CGI_HPP
#define CGI_HPP

#include "webserv.hpp"
#include "Request.hpp"
#include "config_parser.hpp"
#include "Webserver.hpp"
#include "Connection.hpp"


class CGI
{
    private:


    public:

    static int run_cgi(Request & request, configParser::ServerConfig & server_block, Webserver & webserver, Connection *conn);
    static std::string  construct_script_path(Request & request, configParser::ServerConfig & server_block);
    static char **construct_argv(const char* &script_path);
    static char **construct_envp(Request& request, configParser::ServerConfig & server_block);

};

#endif

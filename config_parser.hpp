#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

# include <iostream>
# include "webserv.hpp"


class configParser
{
    private:

    public:
    struct LocationConfig
    {
        std::string path;
        std::string root;
        int autoindex; //1 = on, 0 = off
        int allowed_methods_present;
        std::vector<std::string> allowed_methods;
        int redirection_present;  //1 = on, 0 = off
        std::string path_redirection;
        std::string redirection_code;

        LocationConfig() : autoindex(0), allowed_methods_present(0), redirection_present(0) {}
    };

    struct ServerConfig
    {
        std::string host;
        int port;
        std::string server_name;
        std::string root;
        int client_max_body_size;
        std::string error_code;
        std::string path_error_page;
        std::map<std::string, std::string> error_pages_map;
		bool uploads_location_present;

        ServerConfig(): client_max_body_size(0), uploads_location_present(false) {}

        std::vector<LocationConfig> locations;
    };

    std::vector<ServerConfig> serverConfigVector;
    std::vector<std::string> tokenize(std::string config_file);
    int parse_server_block(std::vector<std::string> &tokens);
    int parse_location_block(std::vector<std::string> &tokens, size_t &i, ServerConfig &currentServer);

};
std::ostream& operator<<(std::ostream& os, const configParser::ServerConfig& config);




#endif

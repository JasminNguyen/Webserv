#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "webserv.hpp"
#include <ostream> // for std::ostream



class configParser
{
    private:

    public: 
    struct LocationConfig
    {
        std::string path;
        std::string root;
        int autoindex; //1 = on, 0 = off
        std::vector<std::string> allowed_methods;

        LocationConfig() : autoindex(0) {} 
    };

    struct ServerConfig
    {
        std::string host;
        int port;
        std::string root;

        std::vector<LocationConfig> locations;
    };
    std::vector<ServerConfig> serverConfigVector;
    std::vector<std::string> tokenize(std::string config_file);
    int parse_server_block(std::vector<std::string> &tokens);
    int parse_location_block(std::vector<std::string> &tokens, size_t &i, ServerConfig &currentServer);
    static int config_parser(std::string config_file, std::vector<ServerConfig> &serverConfigVector);
    static int parse_location(std::istringstream &ss);

};
std::ostream& operator<<(std::ostream& os, const configParser::ServerConfig& config);




#endif
#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "webserv.hpp"
#include <ostream> // for std::ostream



struct LocationConfig
{
    std::string path;
    std::string root;
};

struct serverConfig
{
    std::string host;
    int port;
    std::string root;

    std::vector<LocationConfig> locations;
};

//extern std::vector<serverConfig> serverConfigVector; 

std::ostream& operator<<(std::ostream& os, const serverConfig& config);
int config_parser(std::string config_file, std::vector<serverConfig> &serverConfigVector);

#endif
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


std::ostream& operator<<(std::ostream& os, const serverConfig& config)
{
    os << "host=" << config.host << ", port=" << config.port << ", root=" << config.root;
    return os;
}

std::vector<serverConfig> serverConfigVector;

int config_parser(std::string config_file);

#endif
#include "webserv.hpp"
#include "config_parser.hpp"
#include "Exceptions.hpp"


std::ostream& operator<<(std::ostream& os, const configParser::ServerConfig& config)
{
    os << "\n" << " ===== BEGIN SERVER BLOCK ======" << "\n";
    os << "host=" << config.host << ", port=" << config.port << ", root=" << config.root << ", client_max_body_size=" << config.client_max_body_size;
    for(size_t i = 0; i < config.locations.size(); i++)
    {
        os << "\n";
        os << "l_path[" << i << "]=" << config.locations[i].path << ", l_root[" << i << "]=" << config.locations[i].root;
        if(config.locations[i].autoindex)
        {
            os << ", l_autoindex[" << i << "]=" <<config.locations[i].autoindex;
        }
        if(config.locations[i].allowed_methods_present)
        {
            size_t j = 0;
            while(j < config.locations[i].allowed_methods[j].size())
            {
                os << ", l_allowed_methods[" << j << "]=" << config.locations[i].allowed_methods[j];
                j++;
            }
        }
    }
    os << "\n" << " ===== END SERVER BLOCK ======" << "\n";
    return os;
}

int configParser::parse_location_block(std::vector<std::string> &tokens, size_t &i, ServerConfig &currentServer)
{
    
    while(i < tokens.size())
    {
        LocationConfig currentLocation;
        i++;
        currentLocation.path = tokens[i];
        if(tokens[i + 1] == "{")
        {
            while(i < tokens.size() && tokens[i] != "}")
            {
                if(tokens[i] == "root")
                {
                    currentLocation.root = tokens[++i];
                    i++;
                }
                else if(tokens[i] == "autoindex")
                {
                    currentLocation.autoindex = atoi(tokens[++i].c_str());
                    i++;
                }
                else if(tokens[i] == "allowed_methods")
                {
                    currentLocation.allowed_methods_present = 1;
                    while(tokens[i] != ";")
                    {
                        currentLocation.allowed_methods.push_back(tokens[++i]);
                    }
                    i++;
                    
                }
                else
                {
                    i++;
                }
                
            }
            currentServer.locations.push_back(currentLocation);
        } 
        else
        {
            throw Exceptions("Syntax error: opening bracket missing!\n");
        }
        i++;
        if(tokens[i] == "}")
        {
            break;
        }
    }
    
    return 0;
}
int configParser::parse_server_block(std::vector<std::string> &tokens)
{
    size_t i = 0;
    
    while(i < tokens.size())
    {
        ServerConfig currentServer;
        if(tokens[i] == "server" && tokens[i + 1] == "{")
        {
            while(i < tokens.size() && tokens[i] != "}")
            {
                if(tokens[i] == "listen")
                {
                    currentServer.host = tokens[++i];
                    currentServer.port = atoi(tokens[i + 2].c_str());
                    i++;
                }
                else if(tokens[i] == "root")
                {
                    currentServer.root = tokens[++i];
                    i++;
                }
                else if(tokens[i] == "client_max_body_size")
                {
                    currentServer.client_max_body_size = atoi(tokens[++i].c_str());
                    i++;
                }
                else if(tokens[i] == "location")
                {
                    parse_location_block(tokens, i, currentServer);
                }
                else
                {
                    i++;
                }
            }
            serverConfigVector.push_back(currentServer);
        }
        else
        {
            throw Exceptions("Syntax error: opening bracket missing!\n");
        }
        i++;
       
    }
    return 0;
}


std::vector<std::string> configParser::tokenize(std::string config_file)
{
    std::vector<std::string> tokens;
    std::ifstream file(config_file);//opening the file
    std::string line;
    std::string token;
    
    while(getline(file, line))
    {
        std::istringstream ss(line);
        while(ss >> token)
        {
            size_t i = 0;
            while (i < token.length()) 
            {
                if (token[i] == '{' || token[i] == '}' || token[i] == ';' || token[i] == ':') 
                {
                    if (i > 0)
                    {
                        tokens.push_back(token.substr(0, i));  // -> "8080"
                    }
                    tokens.push_back(std::string(1, token[i])); // -> ";" -> creating a string of size of 1 with the character at the end (;, } or {})
                    token = token.substr(i + 1); // chop off what we just processed so we can start from whatever is after the character
                    i = 0; // restart from beginning of new token
                } 
                else 
                {
                    ++i;
                }
               
            } 
            if (!token.empty())
                tokens.push_back(token); // push what remains!
        }
    }
    return tokens;

}

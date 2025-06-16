#include "webserv.hpp"
#include "config_parser.hpp"


std::ostream& operator<<(std::ostream& os, const configParser::ServerConfig& config)
{
    os << "host=" << config.host << ", port=" << config.port << ", root=" << config.root;
    for(size_t i = 0; i < config.locations.size(); i++)
    {
        os << "\n";
        os << "l_path[" << i << "]=" << config.locations[i].path << ", l_root[" << i << "]=" << config.locations[i].root;
        if(config.locations[i].autoindex)
        {
            os << ", l_autoindex[" << i << "]=" <<config.locations[i].autoindex;
        }
        // if(config.locations[i].allowed_methods)
        // {

        // }
        os << "\n";
    }
    return os;
}

// int configParser::parse_location(std::istringstream &ss)
// {
//         bool inside_location = true;
//         std::string token;
//         LocationConfig currentLocation;
//         std::vector<LocationConfig> locations;

//         while(ss >> token)
//         {

//                 currentLocation = LocationConfig();
//                 currentLocation.path = token;
//                 std::cout << "location path: " << currentLocation.path << std::endl;



        
//             if(inside_location && token == "}")
//             {
//                 inside_location = false;
//                 locations.push_back(currentLocation);
//                 continue;
//             }
//             if(inside_location && token == "root")
//             {
//                 ss >> token;
//                 currentLocation.root = token;
//                 std::cout << "location root: " << currentLocation.root<< std::endl;
//             }
//             if(inside_location && token == "autoindex")
//             {
//                 ss >> token;
//                 currentLocation.autoindex = atoi(token.c_str());
//                 std::cout << "location autoindex: " << currentLocation.autoindex << std::endl;
//             }
//             // if(inside_location && token == "allowed_methods")
//             // {
//             //     ss >> token;
//             //     currentLocation.allowed_methods = token; // this could be more than one // implementation in progress
//             // }


//         }
   
//     return 0;
    
// }

// int configParser::config_parser(std::string config_file, std::vector<ServerConfig> &serverConfigVector)
// {
//     bool inside_server = false;
//     ServerConfig currentServer;

//     // 1. TOKENIZE & PARSE
    
//     std::ifstream file(config_file);//opening the file
   
//     std::string line;
//     while(getline(file, line)) //reading line by line from it
//     {
//         std::istringstream ss(line);
//         std::string token;
       
//         while(ss >> token) //splitting the input into tokens where there is a space/newline
//         {
//             //parse into struct
//             //std::cout << "Token: " << token << std::endl; 
//             if(token == "server")
//             {
//                 inside_server = true;
//                 currentServer = ServerConfig(); // calling the constructor
//                 continue;
//             }
//             if (token == "}") 
//             {
//                 if (inside_server) 
//                 {
//                     serverConfigVector.push_back(currentServer);
//                     inside_server = false;
//                 }
//                 continue;
//             }
            
//             if(inside_server && token == "listen")
//             {
//                 std::string value;
//                 ss >> value; // >> skips spaces/newlines, so we essentially move to next token 
                        
//                 size_t colonPos = value.find(':');
//                 currentServer.host = value.substr(0, colonPos);
//                 //std::cout << "host is: " << currentServer.host << std::endl;
//                 size_t portPos = colonPos + 1;
//                 currentServer.port = std::atoi(value.substr(portPos).c_str());
//                 //std::cout << "port is: " << currentServer.port << std::endl;
//             }
//             if(inside_server && token == "root")
//             {
//                 std::string value;
//                 ss >> value;

//                 currentServer.root = trim(value);
//                 //std::cout << "root is: " << currentServer.root << std::endl;
//             }  
//             if(inside_server && token == "location")
//             {
//                 std::cout << "here" << std::endl;
//                 configParser::parse_location(ss);
//                 continue;
//             }   
            
//         }
//     }
//     std::cout << "Parsed " << serverConfigVector.size() << " server blocks.\n";
//     return 0;
// }


int configParser::parse_location_block(std::vector<std::string> &tokens, size_t &i, ServerConfig &currentServer)
{
    
    while(i < tokens.size())
    {
        LocationConfig currentLocation;
        i++;
        currentLocation.path = tokens[i];
        if(tokens[i + 1] == "{")
        {
             //std::cout << "location path is: " << currentLocation.path << std::endl;
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
                else
                {
                    i++;
                }
                
            }
            currentServer.locations.push_back(currentLocation);
        }
        i++;
       
        
    }
    
    return 0;
}
int configParser::parse_server_block(std::vector<std::string> &tokens)
{
    size_t i = 0;
    ServerConfig currentServer;

    while(i < tokens.size())
    {
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
        i++;
       
    }
   
    return 0;
}

//still acting weird -> why autoindex twice?

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

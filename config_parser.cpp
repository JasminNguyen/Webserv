#include "webserv.hpp"
#include "config_parser.hpp"
#include "Exceptions.hpp"


std::ostream& operator<<(std::ostream& os, const configParser::ServerConfig& config)
{
    os << "\n" << " ===== BEGIN SERVER BLOCK ======" << "\n";
    os << "host=" << config.host << ", port=" << config.port <<  ", server_name=" << config.server_name <<  ", root=" << config.root << ", client_max_body_size=" << config.client_max_body_size;
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
					i++;
                    while(tokens[i] != ";")
                    {
                        currentLocation.allowed_methods.push_back(tokens[i]);
						//std::cout << "METHOD: " << tokens[i] << std::endl;
						if(tokens[i] != "DELETE" && tokens[i] != "POST" && tokens[i] != "GET") // check if we have anything other than the 3 methods
						{
							throw Exceptions("Error: Unauthorised Method!");
						}
						i++;
                    }
                    i++;

                }
                else if(tokens[i] == "return")
                {
                    currentLocation.redirection_code = tokens[++i];
                    currentLocation.path_redirection = tokens[++i];
                    currentLocation.redirection_present = 1;
                    //std::cout << "path redirection in parser: " << currentLocation.path_redirection << std::endl;
                    // if(currentLocation.redirection_present == 1)
                    // {
                    //     std::cout << "In parser we have the redirection_present set to 1" << std::endl;
                    // }
                    i++;
                }
                else if(tokens[i] == "/cgi-bin")
                {
                    while(1)
                    {
                        if(tokens[i] == "root")
                        {
                            currentLocation.path = tokens[++i];
                            // std::cout << "location in parser is here: " << &currentLocation.path << std::endl;
                            break;
                        }
                        i++;
                    }
                }
				else if(tokens[i] == "/uploads")
				{
					currentServer.uploads_location_present = true;
					i++;
				}
                else
                {
                    i++;
                }

            }
			if(currentLocation.allowed_methods_present != 1) //if none are set we just allow all 3
			{
				currentLocation.allowed_methods.push_back("GET");
				currentLocation.allowed_methods.push_back("POST");
				currentLocation.allowed_methods.push_back("DELETE");
			}
            currentServer.locations.push_back(currentLocation);
        }
        i++;
        if(tokens[i] == "}")
        {
            break;
        }
    }
    // std::cout << "number of location blocks in parsing_location_blocks: " << currentServer.locations.size() << std::endl;
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
                else if(tokens[i] == "server_name")
                {
                    currentServer.server_name = tokens[++i];
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
                else if(tokens[i] == "error_page")
                {
                    currentServer.error_code = tokens[++i];
                    currentServer.path_error_page = tokens[++i];
                    currentServer.error_pages_map[currentServer.error_code] = currentServer.path_error_page; //put it in map
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


std::vector<std::string> configParser::tokenize(std::string config_file)
{
    std::vector<std::string> tokens;
    std::ifstream file(config_file.c_str());//opening the file
    std::string line;
    std::string token;
    int opening_brackets = 0;
    int closing_brackets = 0;

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

                    if(token[i] == '{')
                    {
                        opening_brackets++;
                    }
                    else if(token[i] == '}')
                    {
                        closing_brackets++;
                    }
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
    if(opening_brackets != closing_brackets)
    {
        throw Exceptions("Syntax error: bracket mismatch!\n");
    }
    return tokens;

}

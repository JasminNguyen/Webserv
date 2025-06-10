#include "webserv.hpp"
#include "config_parser.hpp"


std::ostream& operator<<(std::ostream& os, const serverConfig& config)
{
    os << "host=" << config.host << ", port=" << config.port << ", root=" << config.root;
    return os;
}

int config_parser(std::string config_file, std::vector<serverConfig> serverConfigVector)
{
    bool inside_server = false;
    serverConfig currentServer;

    // 1. TOKENIZE
    
    std::ifstream file(config_file);//opening the file
   
    std::string line;
    while(getline(file, line)) //reading line by line from it
    {
        std::istringstream ss(line);
        std::string token;
       
        while(ss >> token) //splitting the input into tokens where there is a space/newline
        {
            //parse into struct
            //std::cout << "Token: " << token << std::endl; 
            if(token == "server")
            {
                inside_server = true;
                currentServer = serverConfig(); // does this call the constructor?
                continue;
            }
            if (token == "}") 
            {
                if (inside_server) 
                {
                    serverConfigVector.push_back(currentServer);
                    inside_server = false;
                }
                continue;
            }
            
            if(inside_server && token == "listen")
            {
                std::string value;
                ss >> value; // >> skips spaces/newlines, so we essentially move to next token 
                        
                size_t colonPos = value.find(':');
                currentServer.host = value.substr(0, colonPos);
                std::cout << "host is: " << currentServer.host << std::endl;
                size_t portPos = colonPos + 1;
                currentServer.port = std::atoi(value.substr(portPos).c_str());
                std::cout << "port is: " << currentServer.port << std::endl;
            }
            if(inside_server && token == "root")
            {
                        std::string value;
                        ss >> value;

                        currentServer.root = trim(value);
                        std::cout << "root is: " << currentServer.root << std::endl;
            }         

        }
    }

    return 0;
}

//end goal:
//create a vector of structs for each server 
//error check if something is missing
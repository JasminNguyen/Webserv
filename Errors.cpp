#include <iostream>
#include "Connection.hpp"
#include "config_parser.hpp"

std::string find_error_message(int error_code)
{
    switch (error_code)
    {
    case 403:
        return "Forbidden";
        break;
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 413:
        return "Payload Too Large";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown Error";
    }
}
void Connection::generate_error_page(int error_code, configParser::ServerConfig& server)
{
    std::string error_message = find_error_message(error_code);
    //check if there is a provided error page in config file
    if(!server.path_error_page.empty())
    {
        //find correct error page in folder structure
    }
    else
    {
        //generate an error page dynamicallyy
    }
}
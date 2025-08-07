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
        for(std::map<int, std::string>::iterator it = server.error_pages_map.begin(); it != server.error_pages_map.end(); it++)
        {
            if(error_code == it->first) //found matching right error code in the config file
            {
                //find the path to the error page -> DO I PUT THE PATH TO THE ERROR PAGE IN SOURCE?
                this->get_source().set_path(it->second);
                //DO I SET THE STATUS CODE/message/etc HERE AS WELL?
                this->get_response().set_status_code() = error_code;
                this->get_response().set_status_string() = error_message;

                return;
            }
        }
    }
    else
    {
        //generate an error page dynamicallyy
       std::ostringstream oss;
        oss << "<html><head><title>" << error_code << " " << error_message << "</title></head>\n"
        << "<body>\n"
        << "    <center><h1>" << error_code << " " << error_message << "</h1></center>\n"
        << "    <hr>\n"
        << "    <center>webserv</center>\n"
        << "</body>\n"
        << "</html>";

        this->get_response().set_body() = oss.str();
        this->get_response().set_status_code() = error_code;
        this->get_response().set_status_string() = error_message;
    }
}
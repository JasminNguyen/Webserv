#include <iostream>
#include "Connection.hpp"
#include "config_parser.hpp"

std::string find_error_message(std::string error_code)
{
    if (error_code == "403") {
        return "Forbidden";
    } else if (error_code == "404") {
        return "Not Found";
    } else if (error_code == "405") {
        return "Method Not Allowed";
    } else if (error_code == "413") {
        return "Payload Too Large";
    } else if (error_code == "500") {
        return "Internal Server Error";
    } else {
        return "Unknown Error";
    }
}

void Connection::generate_error_page(Webserver &webserv, std::string error_code, configParser::ServerConfig& server)
{
    std::string error_message = find_error_message(error_code);
    //check if there is a provided error page in config file
    if(!server.path_error_page.empty())
    {
        std::cout << "error_code: " << error_code << std::endl;
        //find correct error page in folder structure
        for(std::map<std::string, std::string>::iterator it = server.error_pages_map.begin(); it != server.error_pages_map.end(); it++)
        {
            std::cout << "it->first: " << it->first << std::endl;
            std::cout << "it->second: " << it->second << std::endl;
            if(error_code == it->first) //found matching right error code in the config file
            {
                std::cout << "We are using a local error page!" << std::endl;
                // //find the path to the error page -> DO I PUT THE PATH TO THE ERROR PAGE IN SOURCE?
                // this->get_source().set_path(it->second);
                // //DO I SET THE STATUS CODE/message/etc HERE AS WELL?
                // this->get_response().set_status_code(error_code);
                // this->get_response().set_status_string(error_message);
                // return;
                std::string error_path = it->second; // e.g. "/errors/404.html"
                std::cout << "error_path: " << error_path << std::endl;
                error_path = "." + error_path; // quick mapping to ./errors/404.html
                struct stat st;
                if (stat(error_path.c_str(), &st) == 0 && S_ISREG(st.st_mode) &&
                    access(error_path.c_str(), R_OK) == 0)
                {
                    int fd = open(error_path.c_str(), O_RDONLY);
                    if (fd != -1) {
                        this->get_source().set_path(error_path);
                        this->get_source().set_fd(fd);
                        webserv.add_connection_to_poll(fd);

                        // don't forget: caller must still register fd with poll -> handle_request()
                        return;
                    }
                }
            }
        }
    }
    else
    {
        std::cout << "We are generating an error page dynamically!" << std::endl;
        //generate an error page dynamicallyy
       std::ostringstream oss;
        oss << "<html><head><title>" << error_code << " " << error_message << "</title></head>\n"
        << "<body>\n"
        << "    <center><h1>" << error_code << " " << error_message << " (Not local, Dynamic)" << "</h1></center>\n"
        << "    <hr>\n"
        << "    <center>webserv</center>\n"
        << "</body>\n"
        << "</html>";

        this->get_response().set_body(oss.str());
        this->get_response().set_status_code(error_code);
        this->get_response().set_status_string(error_message);
    }
}

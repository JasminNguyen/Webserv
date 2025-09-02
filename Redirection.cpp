#include "webserv.hpp"
#include "Connection.hpp"
#include "Response.hpp"

std::string find_redirection_message(std::string redirection_code)
{
    if(redirection_code == "301")
    {
        return "Permanently Moved";
    }
    else if( redirection_code == "302")
    {
        return "Found";
    }
    else 
    {
        return "What kind of redirection code is this?";
    }
}
/*server response that will be send back to the client so that the client can make a new request based on this*/
void Connection::generate_redirection_response_from_server(configParser::ServerConfig& server)
{
    std::string location_header = server.locations[this->get_location_block_index()].path_redirection; //get the path to the new site (called Location Header)
    std::string redirection_message = find_redirection_message(server.locations[this->get_location_block_index()].redirection_code);
    std::ostringstream response;
    response << "HTTP/1.1 " << server.locations[this->get_location_block_index()].redirection_code <<  " " << redirection_message << "\r\n"; // status line
    response << "Location: " << location_header << "\r\n"; // location header
    response << "\r\n"; // end of headers (blank line, no body)

    //what I just put together needs to be added to the header of the response (no body needed). how will I do that though????? -> just in raw? -> SET_raw()????
    this->get_response().set_raw(response.str());
}
//no generate_new_client_call() needed since the client automatically sends out a new request
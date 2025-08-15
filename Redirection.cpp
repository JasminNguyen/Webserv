#include "webserv.hpp"
#include "Connection.hpp"
#include "Response.hpp"

/*server response that will be send back to the client so that the client can make a new request based on this*/
void Connection::generate_redirection_response_from_server(configParser::ServerConfig& server)
{
    std::string location_header = server.locations[this->get_location_block_index()].path_redirection; //get the path to the new site (called Location Header)
    std::ostringstream response;
    response << "HTTP/1.1 301 Moved Permanently\r\n"; // status line
    response << "Location: " << location_header << "\r\n"; // location header
    response << "\r\n"; // end of headers (blank line, no body)

    //what I just put together needs to be added to the header of the response (no body needed). how will I do that though????? -> just in raw? -> SET_raw()????
    this->get_response().get_raw(response.c_str());
}
//no generate_new_client_call() needed since the client automatically sends out a new request
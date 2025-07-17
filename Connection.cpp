#include "Connection.hpp"
#include "webserv.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"
#include "Webserver.hpp"
#include <iostream>
#include "CGI.hpp"

Connection::Connection(Socket &sock) : _sock(sock) {}
Connection::~Connection() {}

Connection &Connection::operator=(const Connection &ref) {
	if (this != &ref) {
		this->_sock = ref._sock;
		this->_request = ref._request;
		this->_response = ref._response;
		this->_source = ref._source;
	}
	return *this;
}

Socket	&Connection::get_socket() {
	return this->_sock;
}

std::vector<configParser::ServerConfig>	&Connection::getServers() {
			return this->_servers;
		}

Request	*Connection::get_request() {
	return this->_request;
}

Response	*Connection::get_response() {
	return this->_response;
}

Source	*Connection::get_source() {
	return this->_source;
}

int	&Connection::get_port() {
	return this->_port;
}

std::string	&Connection::get_host() {
	return this->_host;
}

/* detect type of triggered event and facilitate right action */
void	Connection::handle_socket_event(Webserver &webserv, pollfd &poll) {
	if (poll.revents & POLLIN) {
		if (typeid(*this) == typeid(ListeningSocket)) {
			this->accept_request(webserv);
		} else {
			this->handle_request(webserv);
		}
	} else {
		this->send_response();
	}
}

/* read from connection source and append to connection response */
void	Connection::handle_source_event(Webserver &webserver, pollfd &poll) {
	char buf[1024];
	int src_fd = this->_source->get_fd();

	if (poll.revents & POLLIN) {
		// read from source and pass into response instance
		int n = read(src_fd, buf, 1024);
		this->_response->get_body().append(buf);
		if (n == 0) {
			// generate response parts
			this->_response->get_http_version() = "HTTP /1.1";
			this->_response->get_status_code() = 200;
			this->_response->get_status_string() = "OK";
			this->_response->get_headers() = this->_request->get_headers();
			webserver.remove_from_poll(src_fd);
			// close source fd
			close(src_fd);
			this->_response->assemble();
		}
	} /*else { // if POLLOUT
		// chunk writing to source fd (cgi)
		//note from jassy: so turns out I was wrong: the cgi pipe will give me POLLIN, not POLLOUT (since it's the out_pipe[0])
		-> so we can actually use the same mechanism that we use for the static website (no extra else statement needed)
	}*/


}

void	Connection::add_server(std::vector<configParser::ServerConfig>::iterator it) {
	this->_servers.push_back(*it);
}

/* accept incoming request on listening socket and
create client socket to establish conncetion */
void	Connection::accept_request(Webserver &webserv) {
	int new_fd = accept(this->get_socket().get_fd(), 0, 0); // Are 0s okay or should we provide info?
	Socket *new_sock = new Socket(new_fd);
	Connection *new_con = new Connection(*new_sock);
	// Create new Source object for this connection
    new_con->_source = new Source(); //added by Jasmin -> to be reviewed by Marc
	webserv.get_connections().push_back(*new_con);
	webserv.add_connection_to_poll(new_fd);
}

/* read and process request to produce response */
void	Connection::handle_request(Webserver &webserv) {
	// read into this->_req->_raw
	this->_request->parse();
	// create response
	std::string target = this->_request->get_target();
	if (target.substr(target.size() - 3, target.size() -1).compare(".py") == 0) {
		// static file or cgi
		/*in here we would probably call the cgi -> to be approved by Marc!
		cgi.run_cgi(request, server_block, webserver, this);
		*/
		configParser::ServerConfig server = this->match_location_block(); //please return the right server block
		CGI::run_cgi(*(this->_request), server, webserv, this);

	} else {
		// open static file and add fd to poll vector
			// check if file exists
			// check if file is readable
			// different error pages if file not readable or doesn't exist?
			this->match_location_block();
			std::string file_path = this->_source->get_path();
			if (access(file_path.c_str() , R_OK) == -1) {
				std::cerr << "File doesn't exist or isn't readable." << std::endl;
			} else {
				// open file
				int fd = open(file_path.c_str(), O_RDONLY);
				if (fd == -1) {
					std::cerr << "Opening the file " << file_path.c_str() << " failed." << std::endl;
				}
				// add fd and connection to map
				std::map<Source &, Connection &> map = webserv.get_source_map();
				map[*(this->_source)] = *this;
				// add poll instance to poll vector
				webserv.add_connection_to_poll(fd);
			}
	}
}

/* if response != NULL, assemble response and send to client */
void	Connection::send_response() {
	if (this->_response->get_raw() != "") {
		// send response - chunked writing based on buffer size
	}
}


void	Connection::match_location_block()
{
	// 1. match IP:port to listen (in the config file)
	 /* -> Marcs vector of ServerConfigs in the Connection gives me the respective server blocks in question
	 I just need to consider those for the next steps */

	// 2. find the host header of the request
	std::map<std::string, std::string> &headers = _request->get_headers();
	std::string host_header;
	for(std::map<std::string, std::string>::iterator it =  headers.begin(); it != headers.end(); it++)
	{
		if(it->first == "host" || it->first == "Host")
		{
			host_header = it->second;
			break;
		}
	}
	// 3. match the host header to server block that are the Connection
	std::vector<configParser::ServerConfig> servers_in_question = this->getServers();
	configParser::ServerConfig* matched_server = NULL;
	for(std::vector<configParser::ServerConfig>::iterator it = servers_in_question.begin(); it != servers_in_question.end(); it++)
	{
		if(it->server_name == host_header) //matches!
		{
			matched_server = &(*it);
			break;
		}
	}
	//no match found - no matching server block or no server_name set -> we fall back onto the first server in servers_in_question
	if(matched_server == NULL && !servers_in_question.empty()) //make sure we have the servers_in_question are not empty (we have at least one server in there bevor accessing the first)
	{
		matched_server = &servers_in_question[0];
	}

	// 4. find the right location (if there are any) and match it with the request target
	/*has to be the longest matching location*/
	std::string script_path;
	if(!matched_server->locations.empty()) //check if there are locations
	{
		std::vector<configParser::LocationConfig> locations_in_question = matched_server->locations;
		std::string target_uri = _request->get_target(); //  something like this "/cgi-bin/foo.py?querypahtk"

		bool match_found = false;

		//remove query string
		std::size_t pos_question_mark = target_uri.find('?');
		std::string clean_uri;
		if(pos_question_mark != std::string::npos)
		{
			clean_uri = target_uri.substr(0, pos_question_mark);
		}
		else
		{
			clean_uri = target_uri;
		}

		//going through locations of the respective server block
		size_t best_len = 0;
		configParser::LocationConfig* best_location = NULL;

		for(std::vector<configParser::LocationConfig>::iterator it = locations_in_question.begin(); it != locations_in_question.end(); it++)
		{
			// Try to find best location match (longest prefix match)
			std::string location_path = it->path;

			if(clean_uri.compare(0, location_path.length(), location_path) == 0) // Does the URI start with the location path?
			{
				if (location_path.length() > best_len)
				{
					best_len = location_path.length();
					best_location = &(*it);
				}
			}
		}

		if (best_location)
		{
			// Append the rest of the URI (after the location path) to the root
			std::string relative_path = clean_uri.substr(best_location->path.length()); //start wher best_location ends until the end of the clean_uri and make a substring out of it 
			script_path = best_location->root + relative_path;
			_source->set_path(script_path); //setting the constructed script path in Source
			match_found = true;
		}
		else
		{
			std::cerr << "No match found in the location blocks" << std::endl;
			// 	_response->set_status_code("404");
			// _response->set_body("Not Found");
			throw std::runtime_error("No match found in location blocks");
		}

	}
}

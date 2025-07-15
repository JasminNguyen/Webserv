#include "Connection.hpp"
#include "webserv.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"
#include "Webserver.hpp"
#include <iostream>

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
			webserver.remove_from_poll(src_fd);
			// close source fd
			close(src_fd);
			// generate response parts
			// this->_response->assemble();
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
	} else {
		// open static file and add fd to poll vector
			// check if file exists
			// check if file is readable
			// different error pages if file not readable or doesn't exist?
			if (access(this->_request->get_target().c_str(), R_OK) == -1) {
				std::cerr << "File doesn't exist or isn't readable." << std::endl;
			} else {
				// open file
				int fd = open(this->_request->get_target().c_str(), O_RDONLY);
				if (fd == -1) {
					std::cerr << "Opening the file " << this->_request->get_target().c_str() << " failed." << std::endl;
				}
				// add fd and connection to map
				std::map<Source &, Connection &> map = webserv.get_source_map();
				map[*(this->_source)] = *this;
				// add poll instance to poll vector
				webserv.add_connection_to_poll(fd);
			}

	}

	//Response	*new_res = new Response();
	//this->_res = new_res;
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
		}
	}
	// 3. match the host header to server block that are the Connection
	std::vector<configParser::ServerConfig> _servers_in_question = 
	for(std::vector<configParser::ServerConfig>::iterator it = )

}

#include "Connection.hpp"
#include "webserv.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"
#include "CGI.hpp"
#include "Exceptions.hpp"


Connection::Connection() {
	//std::cout << "Connection default constructor called" << std::endl;
	this->_host = "0.0.0.0";
	this->_port = -1;
	this->_last_active = time(0);
}

Connection::Connection(Socket sock) {
	//std::cout << "Connection data constructor called" << std::endl;
	this->_sock = sock;
	this->_host = "0.0.0.0";
	this->_port = -1;
	this->_last_active = time(0);
}

Connection::Connection(const Connection &ref) {
	//std::cout << "Connection copy constructor called" << std::endl;
	this->_request = ref._request;
	this->_response = ref._response;
	this->_servers = ref._servers;
	this->_sock = ref._sock;
	this->_source = ref._source;
	this->_host = ref._host;
	this->_port = ref._port;
	this->_last_active = time(0);
}

Connection::~Connection() {
	//std::cout << "Connection destructor called" << std::endl;
}

Connection &Connection::operator=(const Connection &ref) {
	//std::cout << "Connection copy assignment operator called" << std::endl;
	if (this != &ref) {
		this->_request = ref._request;
		this->_response = ref._response;
		this->_servers = ref._servers;
		this->_sock = ref._sock;
		this->_source = ref._source;
		this->_host = ref._host;
		this->_port = ref._port;
		this->_last_active = time(0);
	}
	return *this;
}

Socket	&Connection::get_socket() {
	return this->_sock;
}

std::vector<configParser::ServerConfig>	&Connection::getServers() {
			return this->_servers;
		}

Request	&Connection::get_request() {
	return this->_request;
}

Response	&Connection::get_response() {
	return this->_response;
}

Source	&Connection::get_source() {
	return this->_source;
}

int	&Connection::get_port() {
	return this->_port;
}

std::string	&Connection::get_host() {
	return this->_host;
}

void	Connection::setPort(int port) {
	this->_port = port;
}

void	Connection::setHost(std::string host) {
	this->_host = host;
}

// /* detect type of triggered event and facilitate right action */
// int	Connection::handle_socket_event(Webserver &webserv, pollfd &poll) {
// 	//std::cout << "revents: " << (poll.revents == POLLOUT ? "YES" : "NO") << std::endl;
// 	//std::cout << "poll fd: " << poll.fd << std::endl;
// 	if (poll.revents & POLLIN) {
// 		std::cout << "HERE WE GO with POLLIN!" << std::endl;
// 		if (this->get_socket().get_type() == "Listening Socket") {
// 			std::cout << "HERE WE GO with Listening Socket!" << std::endl;
// 			this->accept_request(webserv);
// 			return 0;
// 		} else {
// 			std::cout << "HERE WE GO with client socket!" << std::endl;
// 			this->handle_request(webserv);
// 			return 0;
// 		}
// 	} else {
// 		std::cout << "HERE WE GO with POLLOUT!" << std::endl;
// 		return this->send_response(webserv);
// 	}
// }

/* read from connection source and append to connection response */
int	Connection::read_from_source(Webserver &webserver, pollfd &poll) {
	char buf[1024];
	int src_fd = this->_source.get_fd();
	if (poll.revents & POLLIN) {
			std::cout << "WE are trying to read from the source" << std::endl;
		// read from source and pass into response instance
		int n = 0;
		while((n = read(src_fd, buf, 1024)) && n == 1024)
		{
			this->_response.get_body().append(buf);
		}
		if(n < 0)
		{
			throw Exceptions("Error: read failed in read_from_source\n");
		}
		else
		{
			buf[n] = 0;
			this->_response.get_body().append(buf);
		}



		// generate response parts
		// this->_response.get_http_version() = "HTTP /1.1";
		// this->_response.get_status_code() = "200";
		// this->_response.get_status_string() = "OK";
		if (this->_response.get_http_version().empty())
			this->_response.get_http_version() = "HTTP/1.1"; // fix extra space

		if (this->_response.get_status_code().empty()) {
			this->_response.get_status_code() = "200";
			this->_response.get_status_string() = "OK";
		}
		if (this->get_value_from_map("Connection") == "close")
			this->_response.get_headers()["Connection"] = "close";
		//generate headers
		this->_response.get_headers()["Connection"] = "close";
		//this->_response.get_headers() = this->_request.get_headers();
		// close source fd
		close(src_fd);
		// remove fd from pollfd vector
		webserver.remove_from_poll(src_fd);
		std::cout << "How often???????????????????????" << std::endl;
		this->_response.assemble();
		// remove pair from source map
		webserver.remove_from_source_map(&(this->_source));
		webserver.add_pollout_to_socket_events(this->get_socket().get_fd());
		return 1;

	}
	return 0;/*else { // if POLLOUT
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
	std::cout << "Accept request" << std::endl;
	if (new_fd < 0) {
		std::cout << "We have a problem." << std::endl;
	}
	Connection new_con = Connection(*this);
	new_con._sock = Socket(new_fd);
	webserv.get_connections().push_back(new_con);
	webserv.add_connection_to_poll(new_fd);
}



std::string	Connection::generate_directory_listing(std::string &file_path)
{
		//open directory
		DIR *d;
		d = opendir(file_path.c_str());
		if(!d)
		{
			return "<html><body><h1>Unable to open directory</h1></body></html>";
		}
		//putting response together
		std::stringstream html;
		html << "<html><head><title>Index of " << file_path << "</title></head><body>";
		html << "<h1>Index of " << file_path << "</h1>";
		html << "<ul>";
		struct dirent *entry;

      	while ((entry = readdir(d)) != NULL)
		{
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}
        	html << " \n" << entry->d_name; //print all directory name
			html << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
      	}
		html << "</ul></body></html>";
      	closedir(d); //close directory
		return html.str(); //return what I just put together
}


void	Connection::setLocationBlockIndex(int location_block_index)
{
	this->_location_block_index = location_block_index;

}

int&	Connection::get_location_block_index()
{
		return this->_location_block_index;
}

int		Connection::has_index_file(const std::string& dir_path, const std::string& index_file_name)
{
    struct stat st;
    std::string index_file_path = dir_path + "/" + index_file_name;

    // Check if it exists and is a regular file
    if (stat(index_file_path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        return 1;
    }
    return 0;
}

/* read and process request to produce response */
void	Connection::handle_request(Webserver &webserv) {
	char buf[1024];
	// read into this->_req->_raw
	std::cout << "handle request on client socket" << std::endl;
	size_t n = 0;
	while ((n = read(this->_sock.get_fd(), buf, sizeof(buf))) && n == 1024) {
		this->_request.get_raw().append(buf);
	}
	if (n < 0) {
		throw(std::exception());
	} else { // difference between n == 0 and n > 0 ???
		this->_request.get_raw().append(buf);
	}
	std::cout << "We are parsing the request" << std::endl;
	std::cout << std::endl << this->_request.get_raw() << std::endl;
	this->_request.parse();
	// create response
	std::string target = this->_request.get_target();
	//MAYBE WE SHOULD DECIDE HERE WHETHER WE ARE DEALING WITH A REDIRECTION (CGI AND FILES CAN BE AFFECTED) -> MATCH_LOCATION BLOCK HERE.
	configParser::ServerConfig &server = this->match_location_block();
	std::cout << "number of location blocks in request handling is: " << server.locations.size() << std::endl;
	std::cout << "our location block index is: " << this->get_location_block_index() << std::endl;
	std::cout << "location in handle request is here: " << &server.locations[this->get_location_block_index()].path << std::endl;
	//std::cout << "path_redirection in handle_request : " << server.locations[this->get_location_block_index()].path_redirection << std::endl;

	if(!server.locations.empty() && server.locations[this->_location_block_index].redirection_present == 1)
	{
		std::cout << "Do we even go HEEEEEEEEEERRRRRREEEE? " << std::endl;
		std::cout << "We are redirecting" << std::endl;
		this->generate_redirection_response_from_server(server);
		this->_response.assemble();
		webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
		return;
	}
	//CHECK FOR POST, GET, DELETE METHOD

	if (target.size() >= 3 && /*target.substr(target.size() - 3).compare(".py") == 0 */ server.locations[this->get_location_block_index()].path == "/cgi-bin/") {
		std::cout << "Hi from the if block to initiate CGI" << std::endl;
		// static file or cgi
		/*in here we would probably call the cgi -> to be approved by Marc!
		cgi.run_cgi(request, server_block, webserver, this);
		*/
	
		CGI::run_cgi(this->_request, server, webserv, *this);

	} else {
		// open static file and add fd to poll vector
			// check if file exists
			// check if file is readable
			// different error pages if file not readable or doesn't exist?
		std::cout << "target: " << this->_request.get_target() << std::endl;
		//std::cout << "target: " << this->_request.get_target() << std::endl;
		//this->_source.set_path("./content/test.html");
		std::string file_path = this->_source.get_path();
		std::cout << "file path: " << file_path << std::endl;

		struct stat st;
		if (stat(file_path.c_str(), &st) == 0) {
			if (S_ISDIR(st.st_mode)) {
				std::cout << "should be a directory." << std::endl;
				//check if there is an index file in the directory
				if(has_index_file(file_path, "index.html") == 1)
				{
					//serve index file
					file_path.append("/index.html");
				}
				else //no index file -> check if autoindex is on
				{
					if(!server.locations.empty() && server.locations[this->get_location_block_index()].autoindex == 1)
					{
						//generate directory listing
						std::string directory_listing_html = this->generate_directory_listing(file_path); //where do I put the return value
						this->_response.set_body(directory_listing_html); //CORRECT????
						this->_response.set_status_code("200");
						this->_response.set_status_string("OK");
						// generate headers
						this->_response.assemble();
						webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
						return;
					}
					else //autoindex off
					{
						generate_error_page("403", server);
						if (this->_source.get_fd() != -1)
						{
							webserv.add_to_source_map(&(this->_source), this);
							webserv.add_connection_to_poll(this->_source.get_fd());
							return; // body will be read later
						}
						//generate headers
						this->_response.assemble();
						webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
						return;
					}
				}
			}
			if (access(file_path.c_str() , R_OK) == -1) {
				std::cerr << "File doesn't exist or isn't readable." << std::endl;
				if (errno == EACCES)
				{
					generate_error_page("403", server);
					if (this->_source.get_fd() != -1)
						{
							webserv.add_to_source_map(&(this->_source), this);
							webserv.add_connection_to_poll(this->_source.get_fd());
							return; // body will be read later
						}
				}
				else if (errno == ENOENT)
				{
					generate_error_page("404", server);
					if (this->_source.get_fd() != -1)
						{
							webserv.add_to_source_map(&(this->_source), this);
							webserv.add_connection_to_poll(this->_source.get_fd());
							return; // body will be read later
						}
				}
				else
				{
					generate_error_page("500", server);
					if (this->_source.get_fd() != -1)
						{
							webserv.add_to_source_map(&(this->_source), this);
							webserv.add_connection_to_poll(this->_source.get_fd());
							return; // body will be read later
						}
				}
				//generate headers
				this->_response.assemble();
				webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
				return;

			} else {
				// open file
				std::cout << "Am I a file?" << std::endl;
				int fd = open(file_path.c_str(), O_RDONLY);
				if (fd == -1) {
					std::cerr << "Opening the file " << file_path.c_str() << " failed." << std::endl;
				}
				this->_source.set_fd(fd);
				// add fd and connection to map
				webserv.add_to_source_map(&(this->_source), this);
				// add poll instance to poll vector
				webserv.add_connection_to_poll(fd);
				std::cout << "Opening static file has worked." << std::endl;
			}
		} else {
			// errno == ENOENT
			// stat() return an error --> 404
			if (errno == ENOENT) {
				generate_error_page("404", server);
				if (this->_source.get_fd() != -1)
				{
					webserv.add_to_source_map(&(this->_source), this);
					webserv.add_connection_to_poll(this->_source.get_fd());
					return; // body will be read later
				}
				//generate headers
				this->_response.assemble();
				webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
				return;
			} else {
				throw(std::runtime_error("stat fails but it's not 404. Let's have a look."));
			}
		}
	}
}




/* if response != "", send to client */
int	Connection::send_response(Webserver &webserv) {
	int fd = this->get_socket().get_fd();
	std::string &response = this->_response.get_raw();
	if (response != "") {
		std::cout << "trying to send.." << std::endl;
		//std::cout << "Response: " << response << std::endl;
		// send response - chunked writing based on buffer size
		//std::cout << "fd: " << fd << std::endl;
		size_t n = write(fd, response.c_str(), response.size());
		//response = "";
		if (n < 0) {
			throw(std::exception());
		} else {
			if (n == response.size()) {
				std::cout << "Response delivered. YAY!!!" << std::endl;
			} else {
				std::cout << "Response not fully delivered." << std::endl;
			}
			close(fd);
			webserv.remove_from_poll(fd);
			// if request has Connection: close"
			if (this->get_value_from_map("Connection") == "close") {
				close(fd);
				webserv.remove_from_poll(fd);
			}
			else {
				webserv.remove_pollout_from_socket_events(fd);
			}
			//this->reset_revents(webserv, fd);
			(void)webserv;
			std::cout << "Response:" << std::endl << std::endl;
			std::cout << response << std::endl;
			return 1;
			// remove connection (this) from webserv - is that possible right here?? We are in a method on that instance
		}
	} else {
		std::cout << "Response is still empty right now" << std::endl;
		return 0;
	}
}

configParser::ServerConfig& Connection::match_location_block()
{
	// 1. match IP:port to listen (in the config file)
	 /* -> Marcs vector of ServerConfigs in the Connection gives me the respective server blocks in question
	 I just need to consider those for the next steps */

	// 2. find the host header of the request
	std::map<std::string, std::string> &headers = _request.get_headers();
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
	std::vector<configParser::ServerConfig> &servers_in_question = this->getServers();
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
	std::string target_uri = _request.get_target(); //  something like this "/cgi-bin/foo.py?querypahtk"

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

	if(!matched_server->locations.empty()) //check if there are locations
	{
		std::vector<configParser::LocationConfig> &locations_in_question = matched_server->locations;

		//going through locations of the respective server block
		size_t best_len = 0;
		int best_index = INT_MAX;
		size_t idx = 0;
		configParser::LocationConfig* best_location = NULL;

		for(std::vector<configParser::LocationConfig>::iterator it = locations_in_question.begin(); it != locations_in_question.end(); it++, idx++)
		{
			// Try to find best location match (longest prefix match)
			std::string location_path = it->path;

			if(clean_uri.compare(0, location_path.length(), location_path) == 0) // Does the URI start with the location path?
			{
				if (location_path.length() > best_len)
				{
					best_len = location_path.length();
					best_location = &(*it);
					best_index = idx;
				}
			}
		}

		if (best_location)
		{
			// Append the rest of the URI (after the location path) to the root
			std::string relative_path = clean_uri.substr(best_location->path.length()); //start wher best_location ends until the end of the clean_uri and make a substring out of it
			script_path = best_location->root + '/' + relative_path;
			_source.set_path(script_path); //setting the constructed script path in Source
			this->setLocationBlockIndex(best_index); //setting the location block index (needed in handle_request)
			std::cout << "location in match_location_block is here: " << &best_location->redirection_present << std::endl;
		}
		else
		{
			std::cerr << "No match found in the location blocks" << std::endl;
			// 	_response->set_status_code("404");
			// _response->set_body("Not Found");
			throw std::runtime_error("No match found in location blocks");
		}

	}
	else //if there are no locations -> we still have to construct the script_path
	{
		script_path = matched_server->root + clean_uri;
		_source.set_path(script_path);

	}
	std::cout << "number of locations in match_location_block: " << matched_server->locations.size() << std::endl;
	return *(matched_server);
}

bool	Connection::listeningSocketTriggered(int poll_fd) {
	int socket_fd = this->get_socket().get_fd();
	std::string socket_type = this->get_socket().get_type();

	if (socket_fd == poll_fd && socket_type == "Listening Socket") {
		return true;
	} else {
		return false;
	}
}

bool	Connection::clientRequestIncoming(pollfd poll) {
	int socket_fd = this->get_socket().get_fd();

	if (socket_fd == poll.fd && poll.revents & POLLIN) {
		return true;
	} else {
		return false;
	}
}

bool	Connection::clientExpectingResponse(pollfd poll) {
	int socket_fd = this->get_socket().get_fd();

	if (socket_fd == poll.fd && poll.revents & POLLOUT) {
		return true;
	} else {
		return false;
	}
}

bool	Connection::sourceTriggered(int poll_fd) {
	int socket_fd = this->get_socket().get_fd();

	if (socket_fd != poll_fd) {
		return true;
	} else {
		return false;
	}
}

std::string	Connection::get_value_from_map(std::string key) {
	for (std::map<std::string, std::string>::iterator it = this->_request.get_headers().begin(); it != this->_request.get_headers().end(); it++) {
		if (it->first == key) {
			return it->second;
		}
	}
	return "";
}

void Connection::reset_revents(Webserver &webserv, int fd) {
	for (std::vector<pollfd>::iterator it = webserv.get_polls().begin(); it != webserv.get_polls().end(); it ++) {
		if (it->fd == fd) {
			it->revents = 0;
			return;
		}
	}
}

void	Connection::set_time_stamp() {
	if (time(&this->_last_active) == -1) {
		throw(std::runtime_error("time() call failed"));
	}
}

bool	Connection::is_timed_out() {
	if (time(0) - this->_last_active > TIME_OUT) {
		return true;
	} else {
		return false;
	}
}

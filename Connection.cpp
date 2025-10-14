#include "Connection.hpp"
#include "webserv.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"
#include "CGI.hpp"
#include "Exceptions.hpp"

const std::map<std::string, std::string> Connection::mime_types = Connection::init_mime_types();

const std::map<std::string, std::string>	Connection::init_mime_types() {
	std::map<std::string, std::string> m;
    m[".html"] = "text/html";
    m[".htm"]  = "text/html";
    m[".css"]  = "text/css";
    m[".js"]   = "application/javascript";
    m[".json"] = "application/json";
    m[".png"]  = "image/png";
    m[".jpg"]  = "image/jpeg";
    m[".jpeg"] = "image/jpeg";
    m[".gif"]  = "image/gif";
    m[".txt"]  = "text/plain";
    m[".pdf"]  = "application/pdf";
    m[".ico"]  = "image/x-icon";
    return m;
}

Connection::Connection() {
	//std::cout << "Connection default constructor called" << std::endl;
	this->_host = "0.0.0.0";
	this->_port = -1;
	this->_last_active = time(0);
	this->_location_block_index = -1;
}

Connection::Connection(Socket sock) {
	//std::cout << "Connection data constructor called" << std::endl;
	this->_sock = sock;
	this->_host = "0.0.0.0";
	this->_port = -1;
	this->_last_active = time(0);
	this->_location_block_index = -1;
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
	this->_location_block_index = -1;
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
		this->_location_block_index = -1;
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

bool	Connection::_process_uses_cgi() {
	if (this->_source.get_pid()) {
		return true;
	} else {
		return false;
	}
}

bool	Connection::_is_cgi_still_running() {
	int pid = this->_source.get_pid();
	int	status;

	int n = -5;
	n = waitpid(pid, &status, WNOHANG);

	if (n == 0) {
		//std::cout << "CGI still running" << std::endl;
		return true;
	} else {
		return false;
	}
}

/* read from connection source and append to connection response */
int	Connection::read_from_source(Webserver &webserver, pollfd &poll) {

	// if (this->_process_uses_cgi() && !(poll.revents & POLLHUP)) {
	// 	return 0;
	// }

	// char buf[1024];
	// int src_fd = this->_source.get_fd();
	// if (poll.revents & POLLIN) {
	// 		std::cout << "WE are trying to read from the source" << std::endl;
	// 	// read from source and pass into response instance
	// 	int n = 0;
	// 	while((n = read(src_fd, buf, 1024)) && n == 1024)
	// 	{
	// 		this->_response.get_body().append(buf);
	// 	}
	// 	if(n < 0)
	// 	{
	// 		throw Exceptions("Error: read failed in read_from_source\n");
	// 	}
	// 	else
	// 	{
	// 		buf[n] = 0;
	// 		this->_response.get_body().append(buf);
	// 	}
	// }

	char buf[4096];
	ssize_t bytes_read = -2;
	int src_fd = this->_source.get_fd();
	if(poll.revents & POLLIN + POLLHUP)
	{

		bytes_read = read(src_fd, buf, sizeof(buf));

		if (bytes_read >= 0)
		{
			// we got some data, append exactly n bytes
			this->_response.get_body().append(buf, bytes_read);
		}
		else // n < 0: error case
		{
			webserver.remove_from_poll(src_fd);
			return -1;
		}

	}
	if(bytes_read == 0)
	{
		int status;
		int n = 0;

		if(this->get_source().get_pid())
		{
			n = waitpid(this->_source.get_pid(), &status, 0);
			if(n > 0)
			{
				this->_source.set_cgi_finished(true);
			}
		}

		// close source fd
		close(src_fd);
		// remove fd from pollfd vector
		webserver.remove_from_poll(src_fd);
		this->_source.set_fd(-1);

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
		//generate headers
		this->generate_headers();
		if (this->get_value_from_request_map("Connection") == "keep-alive") {
			this->_source.set_path("");
			this->_source.set_pid(0);
		}
		this->_response.assemble();
		webserver.add_pollout_to_socket_events(this->get_socket().get_fd());
		return 1;
	}
	return 0;
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
		html << "<html><head><title>Index of " << file_path << "</title><link rel=\"icon\" href=\"data:,\"></head><body>";
		html << "<h1>Index of " << file_path << "</h1>";
		html << "<ul>";
		struct dirent *entry;
		std::string request_target = this->get_request().get_target();
		if(request_target[request_target.size() - 1] != '/')
		{
			request_target =  this->get_request().get_target() + "/";
		}
      	while ((entry = readdir(d)) != NULL)
		{
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}
        	html << " \n" << entry->d_name; //print all directory name
			html << "<li><a href=\"" << request_target << entry->d_name << "\">" << entry->d_name << "</a></li>";
      	}
		html << "</ul></body></html>";
		this->get_source().set_path(this->get_source().get_path() + "/.html");
		std::cout << "source of directory listing is: " << this->get_source().get_path() << std::endl;
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

void Connection::dismiss_old_request(Webserver &webserv)
{
	std::cout << "we are dismissing the old request" << std::endl;
	size_t fd = this->_source.get_fd();
	close(fd);
	webserv.remove_from_poll(fd);
	this->_source.set_fd(-1);
	std::cout << "source_fd is now: " << this->_source.get_fd() << std::endl;

}

bool	Connection::last_request_process_unfinished() {
	std::cout << "source_fd is: " << this->_source.get_fd() << std::endl;

	if (this->_source.get_fd() != -1) {
		return true;
	} else {
		return false;
	}
}


bool	Connection::request_requires_cgi(configParser::ServerConfig &server) {
	std::string target = this->_request.get_target();

	if (target.size() >= 3 && server.locations[this->get_location_block_index()].path == "/cgi-bin/") {
		return true;
	} else {
		return false;
	}
}

bool	Connection::is_redirection_present(configParser::ServerConfig &server) {

	if(!server.locations.empty() && server.locations[this->_location_block_index].redirection_present == 1) {
		return true;
	} else {
		return false;
	}
}

void	Connection::serve_redirection(Webserver &webserv, configParser::ServerConfig &server) {
	std::cout << "We are redirecting" << std::endl;
		this->generate_redirection_response_from_server(server);
		this->_response.get_headers()["Connection"] = "close";
		this->_response.assemble();
		webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
}


int		Connection::check_content_length_too_big(Webserver &webserv, configParser::ServerConfig & server)
{
		int content_length = 0;
		for(std::map<std::string, std::string>::iterator it = this->_request.get_headers().begin(); it != this->_request.get_headers().end(); it++)
		{
			if(it->first == "Content-Length")
			{
				content_length = atoi(it->second.c_str());
				break;
			}
		}
		if(server.client_max_body_size < content_length)
		{
			this->generate_error_page(webserv, "413", server);
			if (this->_source.get_fd() != -1)
			{
				std::cout << "error in fd" << std::endl;
				return -1;
			}
			//generate headers
			this->generate_headers();
			this->_response.assemble();
			webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
			return 1;
		}
		return 0;
}
void	Connection::create_response(Webserver &webserv, configParser::ServerConfig &server) {
	//CHECK FOR POST, GET, DELETE METHOD

	std::string request_method = this->_request.get_method();

	if (request_method == "GET" || request_method == "POST") {

		if(check_content_length_too_big(webserv, server))
		{
			return;
		}
		if (this->request_requires_cgi(server)) {
			std::cout << "Hi from the if block to initiate CGI" << std::endl;
			CGI::run_cgi(this->_request, server, webserv, *this);
			this->get_source().set_path(this->get_source().get_path() + "/.html");
		} else {
			// open static file
			// open static file and add fd to poll vector
				// check if file exists
				// check if file is readable
				// different error pages if file not readable or doesn't exist?
			std::cout << "target: " << this->_request.get_target() << std::endl;
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
						this->get_source().set_path(this->get_source().get_path() + "/index.html");
						std::cout << "source in directory is: " << this->get_source().get_path() << std::endl;
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
							this->generate_headers();
							this->_response.assemble();
							webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
							return;
						}
						else //autoindex off
						{
							generate_error_page(webserv, "403", server);
							if (this->_source.get_fd() != -1)
							{
								return; // body will be read later
							}
							//generate headers
							this->generate_headers();
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
						generate_error_page(webserv, "403", server);
					}
					else if (errno == ENOENT)
					{
						generate_error_page(webserv, "404", server);
					}
					else
					{
						generate_error_page(webserv, "500", server);
					}
					if (this->_source.get_fd() != -1)
					{
						return; // body will be read later
					}
					//generate headers
					this->generate_headers();
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
					// add poll instance to poll vector
					webserv.add_connection_to_poll(fd);
					std::cout << "Opening static file has worked." << std::endl;
				}
			} else {
				// errno == ENOENT
				// stat() return an error --> 404
				if (errno == ENOENT) {
					generate_error_page(webserv, "404", server);
					if (this->_source.get_fd() != -1)
					{
						return; // body will be read later
					}
					//generate headers
					this->generate_headers();
					this->_response.assemble();
					webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
					return;
				} else {
					throw(std::runtime_error("stat fails but it's not 404. Let's have a look."));
				}
			}
		}
	} /*else if (request_method == "POST") {

	} */ else if (request_method == "DELETE") {
		// delete source if available and
	}
}

bool	Connection::is_method_allowed(configParser::ServerConfig &server) {
	std::vector<std::string>::iterator it;
	for (it = server.locations[this->get_location_block_index()].allowed_methods.begin(); it != server.locations[this->get_location_block_index()].allowed_methods.end(); it++) {
		if (this->_request.get_method() == *it) {
			return true;
		}
	}
	return false;
}

std::string	Connection::get_content_type() {
	std::string file_path = this->_source.get_path();
	std::string file_extension = file_path.substr(file_path.find_last_of("."));
	std::cout << "file extension: " << file_extension << std::endl;
	std::map<std::string, std::string>::const_iterator it = Connection::mime_types.find(file_extension);
	std::string content_type = it->second;
	std::cout << "content type: " << content_type << std::endl;
	return content_type;
}

void	Connection::generate_headers() {
	std::stringstream content_length;
	content_length << this->_response.get_body().size();
	this->_response.set_header("Date", generate_date());
	this->_response.set_header("Server", "RoyalsOfDelay/1.0");
	this->_response.set_header("Content-Length", content_length.str());
	this->_response.set_header("Content-Type", this->get_content_type());
	if (this->get_value_from_request_map("Connection") == "close") {
		this->_response.set_header("Connection", "close");
	} else {
		this->_response.set_header("Connection", "keep-alive");
	}
}

/* read and process request to produce response */
int	Connection::handle_request(Webserver &webserv) {
	if (this->last_request_process_unfinished()) {
		this->dismiss_old_request(webserv);
	}
	if (this->_request.process(this->_sock.get_fd()) == -1) {
		webserv.remove_from_poll(this->_sock.get_fd());
		return -1;
	}
	if(this->_request.get_raw() == "")
	{
		webserv.remove_from_poll(this->_sock.get_fd());
		return -1;
	}
	// create response
	std::string target = this->_request.get_target();
	//MAYBE WE SHOULD DECIDE HERE WHETHER WE ARE DEALING WITH A REDIRECTION (CGI AND FILES CAN BE AFFECTED) -> MATCH_LOCATION BLOCK HERE.
	configParser::ServerConfig &server = this->match_location_block(webserv);
	std::cout << "number of location blocks in request handling is: " << server.locations.size() << std::endl;
	std::cout << "our location block index is: " << this->get_location_block_index() << std::endl;
	std::cout << "location in handle request is here: " << &server.locations[this->get_location_block_index()].path << std::endl;
	//std::cout << "path_redirection in handle_request : " << server.locations[this->get_location_block_index()].path_redirection << std::endl;
	//this->_request.get_method() = "hjb";
	if (this->is_method_allowed(server) == false) {
		generate_error_page(webserv, "405", server);
		if (this->_source.get_fd() != -1)
		{
			return 1; // body will be read later
		}
		//generate headers
		this->generate_headers();
		this->_response.assemble();
		webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
		return 1;
	}

	if(this->is_redirection_present(server)) {
		this->serve_redirection(webserv, server);
		return 1;
	}
	// CHECK FOR POST, GET, DELETE METHOD
	this->create_response(webserv, server);
	return 1;
}

int	Connection::write_to_client(Webserver &webserv) {
	int fd = this->get_socket().get_fd();
	std::string &response = this->_response.get_raw();

	std::cout << "trying to send.." << std::endl;
	//std::cout << "Response: " << response << std::endl;
	// send response - chunked writing based on buffer size
	//std::cout << "fd: " << fd << std::endl;
	std::cout << std::endl;
	std::cout << "FINAL:" << std::endl;
	std::cout << response << std::endl << std::endl;
	size_t n = write(fd, response.c_str(), response.size());
	if (n < 0) {
		close(fd);
		webserv.remove_from_poll(fd);
		this->get_socket().set_fd(-1);
		return -1;
	} else {
		if (n != response.size()) {
			throw Exceptions("Not the whole response was sent.");
		}
		// if request has Connection: close"
		if (this->get_value_from_response_map("Connection") == "close") {
			close(fd);
			webserv.remove_from_poll(fd);
			this->get_socket().set_fd(-1);
		}
		else {
			webserv.remove_pollout_from_socket_events(fd);
			// clean request and response ???

			this->_request.get_raw() = "";
			this->_response.get_raw() = "";
			this->_response.get_body() = "";
			/*
			this->_request.get_body() = "";
			this->_request.get_method() = "";
			this->_request.get_http_version() = "";
			this->_request.get_target() = "";

			this->_response.get_http_version() = "";

			this->_response.get_status_code() = "";
			this->_response.get_status_string() = "";
			*/
			this->get_source().set_fd(-1);
		}
		std::cout << "Response:" << std::endl << std::endl;
		std::cout << response << std::endl;
		return 1;
	}
}


/* if response != "", send to client */
int	Connection::send_response(Webserver &webserv) {
	std::string &response = this->_response.get_raw();
	if (response != "") {
		if (this->write_to_client(webserv) == - 1) {
			return -1;
		} else {
			return 1;
		}
	} else {
		std::cout << "Response is still empty right now" << std::endl;
		return 0;
	}
}

configParser::ServerConfig& Connection::match_location_block(Webserver &webserv)
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
	if(!host_header.empty())
	{
		size_t colon_pos = host_header.find(":");
		//std::cout << "colon_pos is: " << colon_pos << std::endl;
		host_header = host_header.substr(0, host_header.size() - (host_header.size() - colon_pos));
	}
	// 3. match the host header to server block that are the Connection
	std::vector<configParser::ServerConfig> &servers_in_question = this->getServers();
	configParser::ServerConfig* matched_server = NULL;
	std::cout << "size of servers in question is: " << servers_in_question.size() << std::endl;
	for(std::vector<configParser::ServerConfig>::iterator it = servers_in_question.begin(); it != servers_in_question.end(); it++)
	{
		//std::cout << "looking for a server name match" << std::endl;
		//std::cout << "server name is: " << it->server_name << std::endl;
		//std::cout << "host_header is: " << host_header << std::endl;
		if(it->server_name == host_header) //matches!
		{
			std::cout << "WE FOUND A SERVER NAME MATCH" << std::endl;
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
			// 	_response->set_status_code("404");
			// _response->set_body("Not Found");
			//throw std::runtime_error("No match found in location blocks");
			std::cout << "No location block match found - WE ARE GENERATING THE 404 ERROR PAGE" << std::endl;
			this->generate_error_page(webserv, "404", *matched_server);
			//generate headers
			this->generate_headers();
			this->_response.assemble();
			webserv.add_pollout_to_socket_events(this->get_socket().get_fd());
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
	if(socket_fd == poll.fd && poll.revents & POLLIN)
	{
		std::cout << "POLLIN with fd: " << poll.fd << std::endl;
	}
	else
	{
		std::cout << "POLLHUP with fd: " << poll.fd << std::endl;
	}
	if (socket_fd == poll.fd && poll.revents & POLLIN + POLLHUP) {
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

std::string	Connection::get_value_from_request_map(std::string key) {
	for (std::map<std::string, std::string>::iterator it = this->_request.get_headers().begin(); it != this->_request.get_headers().end(); it++) {
		if (it->first == key) {
			return it->second;
		}
	}
	return "";
}

std::string	Connection::get_value_from_response_map(std::string key) {
	for (std::map<std::string, std::string>::iterator it = this->_response.get_headers().begin(); it != this->_response.get_headers().end(); it++) {
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

bool	Connection::is_cgi_broken() {
	int status;

	int n = waitpid(this->_source.get_pid(), &status, WNOHANG);
	if (n == -1) {
		throw Exceptions("Waitpid fails()");
	} else if (n == 0) {
		//std::cout << "CGI is still running" << std::endl;
		return false;
	} else if (n > 0) {
		this->_source.set_cgi_finished(true);
		std::cout << "cgi process is ended" << std::endl;
		if (WIFEXITED(status)) {
			std::cout << "CGI terminated normally" << std::endl;
			return false;
		} else {
			std::cout << "CGI broke" << std::endl;
			return true;
		}
	}
	return false;
}

void	Connection::close_fds() {
	if (this->_sock.get_fd() >= 0) {
		close(this->_sock.get_fd());
		this->_sock.set_fd(-1);
	}
	if (this->_source.get_fd() >= 0) {
		close(this->_source.get_fd());
		this->_source.set_fd(-1);
	}
}

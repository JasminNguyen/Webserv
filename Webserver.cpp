#include "webserv.hpp"
#include "Webserver.hpp"
#include <typeinfo>
#include "Connection.hpp"
#include "ListeningSocket.hpp"
#include "Source.hpp"
#include "Exceptions.hpp"

Webserver::Webserver() {}

Webserver::Webserver(const Webserver &ref) {
	(void)ref;
}

Webserver::~Webserver() {}

Webserver &Webserver::operator=(const Webserver &ref) {
	if (this != &ref) {
		(void)ref;
	}
	return *this;
}

std::vector<configParser::ServerConfig>	&Webserver::get_config() {
	return this->_config;
}

std::vector<Connection>	&Webserver::get_connections() {
	return this->_connections;
}

std::vector<pollfd>	&Webserver::get_polls() {
	return this->_polls;
}

void	Webserver::populate() {
	populate_socket_connections();
	create_polls();
}

/*  */
// check if there already is a socket for that port and host
void	Webserver::populate_socket_connections() {
	for (std::vector<configParser::ServerConfig>::iterator it = this->_config.begin();
	it != this->_config.end(); it++) {
		std::vector<Connection>::iterator con = this->_connection_exists(it);
		if (con == this->_connections.end()) {
			//std::cout << "Create new connection" << std::endl;
			//create new socket, etc.
			//std::cout << "port:  " << it->port << " host: " <<it->host << std::endl;
			ListeningSocket l_sock = ListeningSocket(it->port, it->host);
			Connection new_con = Connection(l_sock);
			new_con.setHost(it->host);
			new_con.setPort(it->port);
			new_con.add_server(it);
			this->_connections.push_back(new_con);
		} else {
			//add to existing connection
			con->add_server(it);
		}
	}
}

std::vector<Connection>::iterator Webserver::_connection_exists(std::vector<configParser::ServerConfig>::iterator config) {
	for (std::vector<Connection>::iterator it = this->_connections.begin(); it != this->_connections.end(); it++) {
		if (config->host == it->get_host() && config->port == it->get_port()) {
			return it;
		}
	}
	return this->_connections.end();
}

/* iterate over Connection vector to create a pollfd instance per
connection and add it to pollfd vector */
void	Webserver::create_polls() {
	for (std::vector<Connection>::iterator it = this->_connections.begin();
	it != this->_connections.end(); it++) {
		int fd = it->get_socket().get_fd();
		//std::cout << "FD: " << fd << std::endl;
		this->add_connection_to_poll(fd);
	}
}

void	Webserver::add_connection_to_poll(int fd) {
	struct pollfd	poll;
	poll.fd = fd;
	poll.events =  POLLIN; // which events do we need to track ???
	poll.revents = 0;
	//unblock_fd(fd);
	this->_polls.push_back(poll);
}

/* remove pollfd instance from pollfd vector */
void	Webserver::remove_from_poll(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			this->_polls.erase(it);
			return ;
		}
	}
}

void	Webserver::remove_connection(Connection *con) {
	for (std::vector<Connection>::iterator it = this->_connections.begin(); it != this->_connections.end(); it++) {
		if (con == &(*it)) {
			this->remove_from_poll(con->get_socket().get_fd());
			this->remove_from_poll(con->get_source().get_fd());
			it->close_fds();
			this->_connections.erase(it);
			return ;
		}
	}
}

Connection *Webserver::get_triggered_connection(int poll_fd) {
	Connection *con = NULL;

	for (std::vector<Connection>::iterator it = this->_connections.begin();
	it != this->_connections.end(); it++) {
		if (it->get_socket().get_fd() == poll_fd || it->get_source().get_fd() == poll_fd) {
			con = &(*it);
			break;
		}
	}
	if (!con) {
		//std::cout << "poll_fd is: " << poll_fd << std::endl;
		//std::cout << "ERROR!!!" << std::endl;
		// INTERNAL ERROR - what to do here if we get here at all ???
		throw Exceptions("Connection not found!");
	}
	return con;
}

int	Webserver::event_router(Connection *con, pollfd poll) {
	int read_status = 0;
	int send_status = 0;
	int status = 0;

	if (con->listeningSocketTriggered(poll.fd)) {
		//std::cout << "POLLIN on LS: " << poll.fd << std::endl;
		con->accept_request(*this);
		return 1;
	} else if (con->clientRequestIncoming(poll)) {
		status = con->handle_request(*this);
		if (status == -1) {
			this->remove_connection(con);
			return 0;
		} else {
			if (status == 1) {
				con->set_time_stamp();
			}
			return 1;
		}
	} else if (con->clientExpectingResponse(poll)) {
		// std::cout << "POLLOUT with fd: " << poll.fd << std::endl;
		// std::cout << "poll revents: " << poll.revents << std::endl;
		send_status = con->send_response(*this);
		if (send_status == 1) {
			// if request has Connection: close
			if (con->get_value_from_response_map("Connection") == "close") {
				this->remove_connection(con);
				return 0;
			} else {
				con->set_time_stamp();
				return 1;
			}
		} else if (send_status == 0) {
			//con->set_time_stamp();
			return 1;
		} else {
			this->remove_connection(con);
			return 0;
		}
	} else if (con->sourceTriggered(poll.fd)) {
		//std::cout << "POLLIN on SOURCE: " << poll.fd << std::endl;
		read_status = con->read_from_source(*this, poll);
		if (read_status == 1) {
			return 1;
		} else if (read_status == -1) {
			this->remove_connection(con);
			return 0;
		} else {
			return 1;
		}
	} else {
		// INTERNAL ERROR - what to do here if we get here at all???
		throw Exceptions("event router doesn't find event.");
	}
}

/* run poll to detect incoming requests on all sockets and perform action */
void	Webserver::launch() {
	Connection	*con;
	while (true) {
		//std::cout << "Do we get here after having an empty response?" << std::endl;
		int n = poll(this->_polls.data(), this->_polls.size(), 100);
		if (n < 0) {
			throw Exceptions("poll() call failed.");
		}
		for (size_t i = 0; i < this->_polls.size() && n > 0; ) {
			if (this->_polls[i].revents & POLLERR + POLLNVAL) {

				//std::cout << "Triggered event: " << this->_polls[i].revents << std::endl;
				pollfd poll = this->_polls[i];
				con = this->get_triggered_connection(poll.fd);
				if(con->_process_uses_cgi() && !con->get_source().get_cgi_finished())
				{
					int status;
					kill(con->get_source().get_pid(), SIGTERM);
					int n = waitpid(con->get_source().get_pid(), &status, 0);
					if (n > 0) {
						con->get_source().set_cgi_finished(true);
					}
					con->get_source().set_pid(0);
					con->get_response().set_body("");
				}
				this->remove_from_poll(poll.fd);
				this->remove_connection(con);
				n--;
			}
			else if (this->_polls[i].revents & POLLIN + POLLOUT + POLLHUP + POLLRDHUP) {
				pollfd poll = this->_polls[i];
				con = this->get_triggered_connection(poll.fd);
				i += this->event_router(con, poll);
				n--;
			} else {
				if (this->_polls[i].revents != 0) {
					// INTERNAL ERROR - should actually never get here - just skip through???
					throw Exceptions("Other event triggered!");
				}
				i++;
			}
		}
		this->_check_for_timeouts();
		this->_check_for_broken_cgi();
	}
}

void	Webserver::parse_config(const char *config_file) {
	configParser configParser;
	std::vector<std::string> tokenVector;

	tokenVector = configParser.tokenize(config_file);
	configParser.parse_server_block(tokenVector);
	this->_config = configParser.serverConfigVector;
	//checking for /uploads location in config file
	for(size_t i = 0; i < this->_config.size(); i++)
	{
		if(configParser.serverConfigVector[i].uploads_location_present == false)
		{
			throw Exceptions("Error: There is a missing '/uploads' location in the configuration file");
		}
	}
	const char* folder = "./uploads";
    if (!(access(folder, F_OK) == 0))
	{
        throw Exceptions("./uploads folder does not exist");
    }
}

void	Webserver::add_pollout_to_socket_events(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			it->events = POLLOUT;
			break;
		}
	}
}

void	Webserver::remove_pollout_from_socket_events(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			it->events = POLLIN;
			break;
		}
	}
}

void	Webserver::listen_to_nothing(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			it->events = 0;
			break;
		}
	}
}


void	Webserver::_check_for_timeouts() {
	int status;
	for (size_t i = 0; i < this->_connections.size(); ) {
		if (this->_connections[i].get_socket().get_type() != "Listening Socket" && this->_connections[i].is_timed_out()) {
			//std::cout << "Connection is timed out!" << std::endl;
			if(this->_connections[i]._process_uses_cgi() && this->_connections[i]._is_cgi_still_running())
			{
				kill(this->_connections[i].get_source().get_pid(), SIGTERM);
				int n = waitpid(this->_connections[i].get_source().get_pid(), &status, 0);
				if (n > 0) {
					this->_connections[i].get_source().set_cgi_finished(true);
				}
				this->_connections[i].get_source().set_pid(0);
				//this->_connections[i].set_time_stamp();
				this->_connections[i].get_response().set_body("");
				configParser::ServerConfig server = this->_connections[i].match_location_block(*this);
				this->remove_from_poll(this->_connections[i].get_source().get_fd());
				if (close(this->_connections[i].get_source().get_fd()) == -1) {
					// INTERNAL ERROR
					throw Exceptions("close call 17 failed.");
					// conn.generate_error_page(webserver, "403", server_block);
					// if (conn.get_source().get_fd() != -1)
					// {
					//     return;
					// }
					// conn.generate_headers();
					// conn.get_response().assemble();
					// webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
					// return;
				}
				this->_connections[i].get_source().set_fd(-1);
				this->_connections[i].generate_error_page(*this, "504", server);
				if (this->_connections[i].get_source().get_fd() != -1)
				{
					return;
				}
				//generate headers
				this->_connections[i].generate_headers();
				this->_connections[i].get_response().assemble();
				this->add_pollout_to_socket_events(this->_connections[i].get_socket().get_fd());
				return;
			}
			this->remove_from_poll(this->_connections[i].get_socket().get_fd());
			this->remove_connection(&(this->_connections[i]));
		} else {
			i++;
		}
	}
}

void	Webserver::_check_for_broken_cgi() {
	for (size_t i = 0; i < this->_connections.size(); ) {
		if (this->_connections[i].get_source().get_pid() && !this->_connections[i].get_source().get_cgi_finished() && this->_connections[i].is_cgi_broken(*this)) {
			//std::cout << "CGI broke!" << std::endl;
			this->remove_from_poll(this->_connections[i].get_source().get_fd());
			this->remove_connection(&(this->_connections[i]));
		} else {
			i++;
		}
	}
}


// close source fd in case of error

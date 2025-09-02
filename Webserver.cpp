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

std::map<Source *, Connection *>	&Webserver::get_source_map() {
	return this->_source_map;
}

void	Webserver::add_to_source_map(Source *key, Connection *value) {
	this->_source_map[key] = value;
}

void	Webserver::remove_from_source_map(Source *key) {
	this->_source_map.erase(key);
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
			std::cout << "Create new connection" << std::endl;
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
	this->_polls.push_back(poll);
}

/* return connection instance that was triggered
to then handle request or send response */
/*
Connection	*Webserver::find_triggered_socket(int poll_fd) {
	for (std::vector<Connection>::iterator con = this->_connections.begin();
	con != this->_connections.end(); con++) {
		if (con->get_socket().get_fd() == poll_fd || con->get_source().get_fd() == poll_fd) {
			return &(*con);
		}
	}
	return NULL;
}
*/

/*
Connection	*Webserver::find_triggered_source(int poll_fd) {
	for (std::map<Source *, Connection *>::iterator it = this->_source_map.begin(); it != this->_source_map.end(); it++) {
		if (it->first->get_fd() == poll_fd) {
			return it->second;
		}
	}
	return NULL;
}
*/

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
		std::cout << "poll_fd is: " << poll_fd << std::endl;
		std::cout << "ERROR!!!" << std::endl;
		throw Exceptions("Connection not found!");
	}
	return con;
}

int	Webserver::event_router(Connection *con, pollfd poll) {
	if (con->listeningSocketTriggered(poll.fd)) {
		std::cout << "POLLIN on LS: " << poll.fd << std::endl;
		con->accept_request(*this);
		return 1;
	} else if (con->clientRequestIncoming(poll)) {
		std::cout << "POLLIN with fd: " << poll.fd << std::endl;
		con->handle_request(*this);
		con->set_time_stamp();
		return 1;
	} else if (con->clientExpectingResponse(poll)) {
		std::cout << "POLLOUT with fd: " << poll.fd << std::endl;
		std::cout << "poll revents: " << poll.revents << std::endl;
		if (con->send_response(*this)) {
			// if request has Connection: close
			if (con->get_value_from_map("Connection") == "close") {
				this->remove_connection(con);
			}
			this->remove_connection(con);
			//con->set_time_stamp();
			return 0;
		} else {
			con->set_time_stamp();
			return 1;
		}
	} else if (con->sourceTriggered(poll.fd)) {
		std::cout << "POLLIN on SOURCE: " << poll.fd << std::endl;
		if (con->read_from_source(*this, poll)) {
			return 0;
		} else {
			return 1;
		}
	} else {
		std::cout << "WHAAAAAAAAAT???" << std::endl;
		throw(std::exception());
	}
}

/* run poll to detect incoming requests on all sockets and perform action */
void	Webserver::launch() {
	Connection	*con;
	while (true) {
		//std::cout << "Do we get here after having an empty response?" << std::endl;
		int n = poll(this->_polls.data(), this->_polls.size(), 100);
		if (n < 0) {
			std::cerr << "Issue with poll" << std::endl;
			throw(std::exception());
		}

		/* for (size_t i = 0; i < this->_connections.size() && n > 0; ) {
			std::cout << "connection[" << i << "]" << this->_connections[i].get_sock_poll()->fd << std::endl;
			if (this->_connections[i].get_sock_poll()->revents & POLLIN ||
			this->_connections[i].get_sock_poll()->revents & POLLOUT) {
				std::cout << "TEST 1" << std::endl;
				con = &this->_connections[i];
				i += this->event_router(con, con->get_sock_poll());
				n--;
				// add new time stamp
			} else if (this->_connections[i].get_source_poll() && this->_connections[i].get_source_poll()->revents & POLLIN) {
				std::cout << "TEST 2" << std::endl;
				con = &this->_connections[i];
				i += this->event_router(con, con->get_source_poll());
				n--;
			} else {
				// check last activity - remove and don't iterate if idle for too long - only iterate if still active
				i++;
			}
		} */

		for (size_t i = 0; i < this->_polls.size() && n > 0; ) {
			if (this->_polls[i].revents & POLLIN || this->_polls[i].revents & POLLOUT) {
				pollfd poll = this->_polls[i];
				con = this->get_triggered_connection(poll.fd);
				i += this->event_router(con, poll);
				/*con = this->find_triggered_socket(this->_polls[i].fd);
				if (con) {
					if (con->handle_socket_event(*this, this->_polls[i])) {
						this->remove_connection(con);
					} else {
						i++;
					}
				} else {
					con = this->find_triggered_source(this->_polls[i].fd);
					if (con) {
						if (con->handle_source_event(*this, this->_polls[i]) == 0)
							i++;
					}
				} */
				// add new time stamp
				n--;
			} else {
				if (this->_polls[i].revents != 0) {
					std::cout << "Triggered event: " << this->_polls[i].revents << std::endl;
				}
				// check last activity - remove and don't iterate if idle for too long - only iterate if still active
				i++;
			}
		}
		this->_check_for_timeouts();
	}
}

void	Webserver::parse_config(const char *config_file) {
	configParser configParser;
	std::vector<std::string> tokenVector;

	tokenVector = configParser.tokenize(config_file);
	configParser.parse_server_block(tokenVector);
	this->_config = configParser.serverConfigVector;
}

void	Webserver::add_pollout_to_socket_events(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			it->events |= POLLOUT;
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

void	Webserver::_check_for_timeouts() {
	for (size_t i = 0; i < this->_connections.size(); ) {
		if (this->_connections[i].get_socket().get_type() != "Listening Socket" && this->_connections[i].is_timed_out()) {
			std::cout << "Connection is timed out!" << std::endl;
			close(this->_connections[i].get_socket().get_fd());
			this->remove_from_poll(this->_connections[i].get_socket().get_fd());
			this->remove_from_source_map(&(this->_connections[i].get_source()));
			this->remove_connection(&(this->_connections[i]));
		} else {
			i++;
		}
	}
}


// close source fd in case of error

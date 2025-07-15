#include "webserv.hpp"
#include "Webserver.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"

Webserver::Webserver() {}
Webserver::~Webserver() {}


std::vector<configParser::ServerConfig>	&Webserver::get_config() {
	return this->_config;
}

std::vector<Connection>	&Webserver::get_connections() {
	return this->_connections;
}

std::vector<pollfd>	&Webserver::get_polls() {
	return this->_polls;
}

std::map<Source &, Connection &>	&Webserver::get_source_map() {
	return this->_source_map;
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
			//create new socket, etc.
			ListeningSocket l_sock = ListeningSocket(it->port, it->host);
			Connection new_con = Connection(l_sock);
			new_con.add_server(it);
			this->_connections.push_back(new_con);
			this->add_connection_to_poll(l_sock.get_fd());
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
		this->add_connection_to_poll(it->get_socket().get_fd());
	}
}

void	Webserver::add_connection_to_poll(int fd) {
	struct pollfd	poll;
	poll.fd = fd;
	poll.events = POLLIN | POLLOUT; // which events do we need to track ???
	this->_polls.push_back(poll);
}

/* return connection instance that was triggered
to then handle request or send response */
Connection	*Webserver::find_triggered_socket(pollfd &poll) {
	for (std::vector<Connection>::iterator con = this->_connections.begin();
	con != this->_connections.end(); con++) {
		if (con->get_socket().get_fd() == poll.fd) {
			return &(*con);
		}
	}
}

Connection	*Webserver::find_triggered_source(pollfd &poll) {
	for (std::map<Source &, Connection &>::iterator it = this->_source_map.begin(); it != this->_source_map.end(); it++) {
		if (it->first.get_fd() == poll.fd) {
			return &(it->second);
		}
	}
}

/* remove pollfd instance from pollfd vector */
void	Webserver::remove_from_poll(int fd) {
	for (std::vector<pollfd>::iterator it = this->_polls.begin(); it != this->_polls.end(); it++) {
		if (it->fd == fd) {
			this->_polls.erase(it);
		}
	}
}

/* run poll to detect incoming requests on all sockets and perform action */
void	Webserver::launch() {
	Connection	*con;
	while (true) {
		int n = poll(this->_polls.data(), this->_polls.size(), 100);

		for (std::vector<pollfd>::iterator poll = this->_polls.begin();
		poll != this->_polls.end() && n > 0; poll++) {
			if (poll->revents & POLLIN | POLLOUT) {
				con = this->find_triggered_socket(*poll);
				if (con) {
					con->handle_socket_event(*this, *poll);
				} else {
					con = this->find_triggered_source(*poll);
					con->handle_source_event(*this, *poll);
				}
				n--;
			}
		}
	}
}

void	Webserver::parse_config() {
	//configParser.parse_server_block(tokenVector)
}

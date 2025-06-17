#include "webserv.hpp"
#include "Webserver.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"

Webserver::Webserver() {}
Webserver::~Webserver() {}

std::vector<Connection>	&Webserver::get_connections() {
	return this->_connections;
}

void	Webserver::populate() {
	create_servers();
	create_connections();
	create_polls();
}

/*  */
void	Webserver::create_servers() {}

/*  */
void	Webserver::create_connections() {}

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
Connection	&Webserver::find_triggered_connection(pollfd &poll) {
	for (std::vector<Connection>::iterator con = this->_connections.begin();
	con != this->_connections.end(); con++) {
		if (con->get_socket().get_fd() == poll.fd) {
			return *con;
		}
	}
}

/* run poll to detect incoming requests on all sockets and perform action */
void	Webserver::launch() {
	while (true) {
		int n = poll(this->_polls.data(), this->_polls.size(), 100);

		for (std::vector<pollfd>::iterator poll = this->_polls.begin();
		poll != this->_polls.end() && n > 0; poll++) {
			if (poll->revents & POLLIN | POLLOUT) {
				Connection	&con = this->find_triggered_connection(*poll);
				con.handle_event(*this, *poll);
				n--;
			}
		}
	}
}

void	Webserver::parse_config() {
	//configParser.parse_server_block(tokenVector)
}

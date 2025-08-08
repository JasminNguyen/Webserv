#include "webserv.hpp"
#include "Webserver.hpp"
#include <typeinfo>
#include "Connection.hpp"
#include "ListeningSocket.hpp"
#include "Source.hpp"

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
	poll.events = POLLIN | POLLOUT; // which events do we need to track ???
	this->_polls.push_back(poll);
}

/* return connection instance that was triggered
to then handle request or send response */
Connection	*Webserver::find_triggered_socket(int poll_fd) {
	for (std::vector<Connection>::iterator con = this->_connections.begin();
	con != this->_connections.end(); con++) {
		if (con->get_socket().get_fd() == poll_fd) {
			return &(*con);
		}
	}
	return NULL;
}

Connection	*Webserver::find_triggered_source(int poll_fd) {
	for (std::map<Source *, Connection *>::iterator it = this->_source_map.begin(); it != this->_source_map.end(); it++) {
		if (it->first->get_fd() == poll_fd) {
			return it->second;
		}
	}
	return NULL;
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
			this->_connections.erase(it);
			return ;
		}
	}
}

Connection *Webserver::get_triggered_connection(int poll_fd) {
	Connection *con;

	con = this->find_triggered_socket(poll_fd);
	if (!con) {
		con = this->find_triggered_source(poll_fd);
	}
	if (!con) {
		std::cout << "ERROR!!!" << std::endl;
	}
	return con;
}

int	Webserver::event_router(Connection *con, pollfd poll) {
	if (con->listeningSocketTriggered(poll.fd)) {
		con->accept_request(*this);
		return 1;
	} else if (con->clientRequestIncoming(poll)) {
		con->handle_request(*this);
		return 1;
	} else if (con->clientExpectingResponse(poll)) {
		if (con->send_response(*this)) {
			this->remove_connection(con);
			return 0;
		} else {
			return 1;
		}
	} else if (con->sourceTriggered(poll.fd)) {
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
		int n = poll(this->_polls.data(), this->_polls.size(), 0);
		if (n < 0) {
			std::cerr << "Issue with poll" << std::endl;
			throw(std::exception());
		}
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
				n--;
			} else {
				i++;
			}
		}
	}
}

void	Webserver::parse_config(const char *config_file) {
	configParser configParser;
	std::vector<std::string> tokenVector;

	tokenVector = configParser.tokenize(config_file);
	configParser.parse_server_block(tokenVector);
	this->_config = configParser.serverConfigVector;
}

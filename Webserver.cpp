#include "webserv.hpp"
#include "Webserver.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"

Webserver::Webserver() {}
Webserver::~Webserver() {}

void	Webserver::populate() {
	create_servers();
	create_connections();
	create_poll();
}

/*  */
void	Webserver::create_servers() {}

/*  */
void	Webserver::create_connections() {}

/* iterate over Connection vector to create a pollfd instance per
connection and add it to pollfd vector */
void	Webserver::create_poll() {
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


/*  */
void	Webserver::launch() {
	int	fd_trig;
	char buf[1024];
	while (true) {

		//run poll to get indicated about triggered sockets
		// timeout shouldn't be negative because we would wait for the next event and stop operating on the other ones
		int n = poll(this->_polls.data(), this->_polls.size(), 100);

		// iterate over poll instances to find triggered fds
		for (std::vector<pollfd>::iterator poll = this->_polls.begin();
		poll != this->_polls.end() && n > 0; poll++) {
			// check if poll instance is triggered
			if (poll->revents & POLLIN | POLLOUT) {
				// find corresponding connection and detect next action on connection
					// accept, recv, send
				for (std::vector<Connection>::iterator con = this->_connections.begin();
				con != this->_connections.end(); con++) {
					if (con->get_socket().get_fd() == poll->fd) {
						if (poll->revents & POLLIN) {
							if (typeid(con) == typeid(ListeningSocket)) {
								// accept() and create new socket, connection and pollfd instance
								int new_client_fd = accept(con->get_socket().get_fd(), 0, 0);
								Socket new_sock = Socket(new_client_fd);
								Connection new_con = Connection(new_sock);
								this->add_connection_to_poll(new_client_fd);
							} else {
								//read()
								read(con->get_socket().get_fd(), buf, 1024);
							}
						} else {
							// send()
							//write(con->get_socket().get_fd(), con->get_response(), con->get_reponse().size());
						}
					}
				}
				n--;	// Through poll before every action ? If yes, should we
						// remember where we left off? fds in the back would have to wait for long
			}
		}
	}
}

void	Webserver::parse_config() {
	//configParser.parse_server_block(tokenVector)
}

#include "Connection.hpp"
#include "webserv.hpp"
#include <typeinfo>
#include "ListeningSocket.hpp"
#include "Webserver.hpp"
#include <iostream>

Connection::Connection(Socket &sock) : _sock(sock) {}
Connection::~Connection() {}

Socket	&Connection::get_socket() {
	return this->_sock;
}

/* detect type of triggered event and facilitate right action */
void	Connection::handle_socket_event(Webserver &webserv, pollfd &poll) {
	if (poll.revents & POLLIN) {
		if (typeid(*this) == typeid(ListeningSocket)) {
			this->accept_request(webserv);
		} else {
			this->handle_request();
		}
	} else {
		this->send_response();
	}
}

/* read from connection source and append to connection response */
void	Connection::handle_source_event(Webserver &webserver, pollfd &poll) {
	char buf[1024];
	int src_fd = this->_src->get_fd();

	if (poll.revents & POLLIN) {
		// read from source and pass into response instance
		int n = read(src_fd, buf, 1024);
		this->_res->get_raw().append(buf);
		if (n == 0) {
			webserver.remove_poll(src_fd);
			// close source fd
			close(src_fd);
		}
	} else { // if POLLOUT
		// chunk writing to source fd (cgi)
	}

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
void	Connection::handle_request() {
	// read into this->_req->_raw
	this->_req->parse();
	// create response
		// static file or cgi
	//Response	*new_res = new Response();
	//this->_res = new_res;
}

/* if response != NULL, assemble response and send to client */
void	Connection::send_response() {
	if (this->_res) {
		this->_res->assemble();
		// send response
	}
}

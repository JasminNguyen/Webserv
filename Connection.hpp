#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Request.hpp"
# include "Response.hpp"
# include "Socket.hpp"
# include "Source.hpp"

class Connection {

	public:

		Connection(Socket &sock);
		~Connection();

		Socket		&get_socket();
		Request		*get_request();
		Response	*get_response();
		Source		*get_source();
		void		handle_socket_event(Webserver &webserver, pollfd &poll);
		void		handle_source_event(Webserver &webserver, pollfd &poll);

	private:

		Connection();

		Socket		&_sock;
		Request		*_request;
		Response	*_response;
		Source		*_source;

		void	accept_request(Webserver &webserv);
		void	handle_request();
		void	send_response();

};

#endif

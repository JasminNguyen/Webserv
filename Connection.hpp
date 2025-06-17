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

		Socket	&get_socket();
		void	handle_event(Webserver &webserver, pollfd &poll);

	private:

		Connection();

		Socket		&_sock;
		Request		*_req;
		Response	*_res;
		Source		*_src;

		void	accept_request(Webserver &webserv);
		void	handle_request();
		void	send_response();

};

#endif

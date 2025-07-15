#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Request.hpp"
# include "Response.hpp"
# include "Socket.hpp"
# include "Source.hpp"
# include "CGI.hpp"
# include "config_parser.hpp"

class Connection {

	public:

		Connection(Socket &sock);
		~Connection();

		Connection &operator=(const Connection &ref);

		Socket		&get_socket();
		Request		*get_request();
		Response	*get_response();
		Source		*get_source();
		int			&get_port();
		std::string	&get_host();

		void		handle_socket_event(Webserver &webserver, pollfd &poll);
		void		handle_source_event(Webserver &webserver, pollfd &poll);
		void		add_server(std::vector<configParser::ServerConfig>::iterator it);
		int			no_duplicates(std::vector<configParser::ServerConfig>::iterator it);

	private:

		Connection();

		Socket		&_sock;
		std::vector<configParser::ServerConfig> _servers;
		Request		*_request;
		Response	*_response;
		Source		*_source;
		int			_port;
		std::string	_host;


		void	accept_request(Webserver &webserv);
		void	handle_request(Webserver &webserv);
		void	send_response();

};

#endif

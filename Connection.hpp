#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "config_parser.hpp"
# include "webserv.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Socket.hpp"
# include "Source.hpp"
# include "Webserver.hpp"

class Connection {

	public:

		Connection();
		Connection(Socket sock);
		Connection(const Connection &ref);
		~Connection();

		Connection 								&operator=(const Connection &ref);

		Socket									&get_socket();
		std::vector<configParser::ServerConfig>	&getServers();
		Request									&get_request();
		Response								&get_response();
		Source									&get_source();
		int										&get_port();
		std::string								&get_host();

		void									setPort(int port);
		void									setHost(std::string host);

		void									handle_socket_event(Webserver &webserver, pollfd &poll);
		void									handle_source_event(Webserver &webserver, pollfd &poll);
		void									add_server(std::vector<configParser::ServerConfig>::iterator it);
		configParser::ServerConfig				&match_location_block(); //finds the right server block or location to serve our static file or cgi

	private:

		Socket									_sock;
		std::vector<configParser::ServerConfig>	_servers;
		Request									_request;
		Response								_response;
		Source									_source;
		int										_port;
		std::string								_host;


		void									accept_request(Webserver &webserv);
		void									handle_request(Webserver &webserv);
		void									send_response();
		void									generate_error_page(int error_code, configParser::ServerConfig& server);

};

#endif

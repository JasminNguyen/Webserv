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
		int										&get_location_block_index(); //ADDED BY JASMIN

		void									setPort(int port);
		void									setHost(std::string host);
		void									setLocationBlockIndex(int location_block_index);//ADDED BY JASMIN

		int										handle_socket_event(Webserver &webserver, pollfd &poll);
		int										read_from_source(Webserver &webserver, pollfd &poll);
		void									add_server(std::vector<configParser::ServerConfig>::iterator it);
		configParser::ServerConfig				&match_location_block(); //finds the right server block or location to serve our static file or cgi
		bool									listeningSocketTriggered(int poll_fd);
		bool									clientRequestIncoming(pollfd poll);
		bool									clientExpectingResponse(pollfd poll);
		bool									sourceTriggered(int poll_fd);
		void									accept_request(Webserver &webserv);
		void									handle_request(Webserver &webserv);
		int										send_response(Webserver &webserv);
		void									generate_error_page(std::string error_code, configParser::ServerConfig& server);
		int										has_index_file(const std::string& dir_path, const std::string& index_file_name);
		std::string								generate_directory_listing(std::string &file_path);
		void									generate_redirection_response_from_server(configParser::ServerConfig& server);

	private:

		Socket									_sock;
		std::vector<configParser::ServerConfig>	_servers;
		Request									_request;
		Response								_response;
		Source									_source;
		int										_port;
		std::string								_host;
		int 									_location_block_index; //ADDED BY JASMIN

};

#endif

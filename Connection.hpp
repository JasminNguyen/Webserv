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

		// int										handle_socket_event(Webserver &webserver, pollfd &poll);
		int										read_from_source(Webserver &webserver, pollfd &poll);
		void									add_server(std::vector<configParser::ServerConfig>::iterator it);
		configParser::ServerConfig				&match_location_block(Webserver &webserv); //finds the right server block or location to serve our static file or cgi
		bool									listeningSocketTriggered(int poll_fd);
		bool									clientRequestIncoming(pollfd poll);
		bool									clientExpectingResponse(pollfd poll);
		bool									sourceTriggered(int poll_fd);
		void									accept_request(Webserver &webserv);
		int										handle_request(Webserver &webserv);
		int										send_response(Webserver &webserv);
		void									generate_error_page(Webserver &webserv, std::string error_code, configParser::ServerConfig& server);
		int										has_index_file(const std::string& dir_path, const std::string& index_file_name);
		std::string								generate_directory_listing(std::string &file_path);
		void									generate_redirection_response_from_server(configParser::ServerConfig& server);
		std::string								get_value_from_request_map(std::string key);
		std::string								get_value_from_response_map(std::string key);
		void									reset_revents(Webserver &webserv, int fd);
		void									set_time_stamp();
		bool									is_timed_out();
		bool									last_request_process_unfinished();
		void									dismiss_old_request(Webserver &webserv);
		int										write_to_client(Webserver &webserv);
		bool									request_requires_cgi(configParser::ServerConfig &server);
		bool									is_redirection_present(configParser::ServerConfig &server);
		void									serve_redirection(Webserver &webserv, configParser::ServerConfig &server);
		void									create_response(Webserver &webserv, configParser::ServerConfig &server);
		bool									is_cgi_broken(Webserver &webserver);
		int										check_content_length_too_big(Webserver &webserv, configParser::ServerConfig & server);
		void									close_fds();
		bool									_process_uses_cgi();
		bool									_is_cgi_still_running();
		bool									is_method_allowed(configParser::ServerConfig &server);
		void									generate_headers();
		std::string								get_content_type();
		static const std::map<std::string, std::string>	init_mime_types();
		int										handle_uploads(Webserver &webserv, configParser::ServerConfig &server);
		bool									header_val_contains_multipart(std::string header_val);
		int										extract_boundary_from_content_type_header(std::string &content, std::string &boundary);
		int										extract_multipart_content_in_request_body(std::string boundary, const std::string &request_body);
		static const std::map<std::string, std::string>	mime_types;
		
		int										write_content_to_uploads_directory();
		std::string								sanitize_filename(std::string filename);
		void									delete_file(Webserver &webserv, configParser::ServerConfig &server);
							
	private:

		Socket									_sock;
		std::vector<configParser::ServerConfig>	_servers;
		Request									_request;
		Response								_response;
		Source									_source;
		int										_port;
		std::string								_host;
		int 									_location_block_index; //ADDED BY JASMIN
		time_t									_last_active;
		std::string								multipart_content;
		std::string								filename;
};

#endif

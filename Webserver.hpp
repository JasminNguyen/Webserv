#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include "webserv.hpp"
# include "config_parser.hpp"

class Connection;
class Source;

class Webserver {

	public:

		Webserver();
		Webserver(const Webserver &ref);
		~Webserver();

		Webserver 								&operator=(const Webserver &ref);

		std::vector<configParser::ServerConfig>	&get_config();
		std::vector<Connection>					&get_connections();
		std::vector<pollfd>						&get_polls();

		void									launch();
		void									parse_config(const char *config_file);
		void									populate();


		void									add_connection_to_poll(int fd);
		void									remove_from_poll(int fd);

		void									remove_connection(Connection *con);

		void 									add_pollout_to_socket_events(int fd);
		void									remove_pollout_from_socket_events(int fd);

	private:

		std::vector<configParser::ServerConfig>	_config;
		std::vector<Connection>					_connections;
		std::vector<pollfd>						_polls;

		void									populate_socket_connections();
		void									create_polls();
		std::vector<Connection>::iterator		_connection_exists(std::vector<configParser::ServerConfig>::iterator it);

		Connection 								*get_triggered_connection(int poll_fd);
		Connection								*find_triggered_socket(int poll_fd);
		Connection								*find_triggered_source(int poll_fd);
		int										event_router(Connection *con, pollfd poll);
		void									_check_for_timeouts();

};

#endif

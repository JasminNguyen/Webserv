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
		std::map<Source, Connection>			&get_source_map();

		void									add_to_source_map(Source key, Connection value);

		void									launch();
		void									parse_config(const char *config_file);
		void									populate();


		void									add_connection_to_poll(int fd);
		void									remove_from_poll(int fd);

	private:

		std::vector<configParser::ServerConfig>	_config;
		std::vector<Connection>					_connections;
		std::vector<pollfd>						_polls;
		std::map<Source, Connection>			_source_map;

		void									populate_socket_connections();
		void									create_polls();
		std::vector<Connection>::iterator		_connection_exists(std::vector<configParser::ServerConfig>::iterator it);

		Connection								*find_triggered_socket(pollfd &poll);
		Connection								*find_triggered_source(pollfd &poll);

};

#endif

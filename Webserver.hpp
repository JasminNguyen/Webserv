#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include "webserv.hpp"
# include "config_parser.hpp"
# include "Connection.hpp"
//# include "Server.hpp"

class Webserver {

	public:

		Webserver();
		~Webserver();

		std::vector<configParser::ServerConfig>	&get_config();
		//std::vector<Server>						&get_servers();
		std::vector<Connection>					&get_connections();
		std::vector<pollfd>						&get_polls();
		std::map<Source &, Connection &>		&get_source_map();



		void	launch();
		void	parse_config();
		void	populate();


		void	add_connection_to_poll(int fd);
		void	remove_from_poll(int fd);

	private:

		std::vector<configParser::ServerConfig>	_config;
		//std::vector<Server>						_servers;
		std::vector<Connection>					_connections;
		std::vector<pollfd>						_polls;
		std::map<Source &, Connection &>		_source_map;

		void	populate_socket_connections();
		void	create_polls();
		//void	create_servers();

		Connection	*find_triggered_socket(pollfd &poll);
		Connection	*find_triggered_source(pollfd &poll);

};

#endif

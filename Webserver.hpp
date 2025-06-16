#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include "webserv.hpp"
# include "config_parser.hpp"
# include "Connection.hpp"
# include "Server.hpp"

class Webserver {

	public:

		Webserver();
		~Webserver();

		void	launch();
		void	parse_config();
		void	populate();

	private:

		std::vector<configParser::ServerConfig>	_config;
		std::vector<Server>						_servers;
		std::vector<Connection>					_connections;
		std::vector<pollfd>						_polls;

		void	create_connections();
		void	create_poll();
		void	create_servers();
		void	add_connection_to_poll(int fd);

};

#endif

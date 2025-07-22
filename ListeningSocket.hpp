#ifndef LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

# include "webserv.hpp"
# include "Socket.hpp"

class ListeningSocket : public Socket {

	public:

		ListeningSocket();
		ListeningSocket(int port, std::string host);
		ListeningSocket(const ListeningSocket &ref);
		~ListeningSocket();

		ListeningSocket	&operator=(const ListeningSocket &ref);

	private:

};

#endif

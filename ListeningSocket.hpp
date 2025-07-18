#ifndef LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

# include "webserv.hpp"
# include "Socket.hpp"

class ListeningSocket : public Socket {

	public:

		ListeningSocket(int port, std::string host);

	private:

};

#endif

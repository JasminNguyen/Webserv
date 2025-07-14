#ifndef LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

# include "Socket.hpp"

class ListeningSocket : public Socket {

	public:

		ListeningSocket::ListeningSocket(int port, std::string host);

	private:

};

#endif

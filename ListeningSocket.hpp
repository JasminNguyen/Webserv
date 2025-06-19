#ifndef LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

# include "Socket.hpp"

class ListeningSocket : public Socket {

	public:

		ListeningSocket(int fd);

		unsigned int	&get_fd();

	private:

		unsigned int	_fd;

};

#endif

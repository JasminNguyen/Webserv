#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket {

	public:

		Socket(int fd);
		~Socket();

		int	get_fd();

	protected:

		unsigned int	_fd;

};

#endif

#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket {

	public:

		Socket();
		Socket(int fd);
		~Socket();

		int	get_fd();

	protected:

		int	_fd;

};

#endif

#ifndef SOCKET_HPP
# define SOCKET_HPP

class Socket {

	public:

		Socket(int fd);
		~Socket();

		int	get_fd();

	private:

		unsigned int	_fd;

};

#endif

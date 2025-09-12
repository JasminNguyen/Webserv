#ifndef SOCKET_HPP
# define SOCKET_HPP

#include "webserv.hpp"

class Socket {

	public:

		Socket();
		Socket(int fd);
		Socket(const Socket &ref);
		~Socket();

		Socket		&operator=(const Socket &ref);

		int			get_fd();
		std::string	get_type();

		void		set_fd(int fd);

	protected:

		int			_fd;
		std::string	_type;

};

#endif

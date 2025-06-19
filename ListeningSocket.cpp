#include "ListeningSocket.hpp"


ListeningSocket::ListeningSocket(int fd) : Socket(fd) {
	this->_fd = fd;
}

unsigned int	&ListeningSocket::get_fd() {
	return this->_fd;
}

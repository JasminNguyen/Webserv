#include "Socket.hpp"

Socket::Socket() {}
Socket::Socket(int fd) : _fd(fd) {}
Socket::~Socket() {}

int	Socket::get_fd() {
	return this->_fd;
}

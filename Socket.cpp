#include "Socket.hpp"

Socket::Socket() : _fd(-1), _type("Socket") {}

Socket::Socket(int fd) : _fd(fd), _type("Socket") {}

Socket::Socket(const Socket &ref) {
	this->_fd = ref._fd;
	this->_type = ref._type;
}

Socket::~Socket() {}

Socket &Socket::operator=(const Socket &ref) {
	if (this != &ref) {
		this->_fd = ref._fd;
		this->_type = ref._type;
	}
	return *this;
}

int	Socket::get_fd() {
	return this->_fd;
}

std::string Socket::get_type() {
	return this->_type;
}

void	Socket::set_fd(int fd) {
	this->_fd = fd;
}

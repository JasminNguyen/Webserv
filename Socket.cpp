#include "Socket.hpp"

Socket::Socket() {
	this->_type = "Socket";
}
Socket::Socket(int fd) : _fd(fd) {
	this->_type = "Socket";
}
Socket::~Socket() {}

int	Socket::get_fd() {
	return this->_fd;
}

std::string Socket::get_type() {
	return this->_type;
}

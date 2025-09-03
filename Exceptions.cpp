#include "webserv.hpp"
#include "Exceptions.hpp"

Exceptions::Exceptions(const std::string& message) : _message(message) {}

Exceptions::~Exceptions() throw() {}

const char* Exceptions::what() const throw() {
    return _message.c_str();
}

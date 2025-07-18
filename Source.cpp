#include "Source.hpp"

const int &Source::get_fd() const {
    return this->_fd;
}

void Source::set_path(std::string path)
{
    this->_path = path;
}

void Source::set_fd(int fd)
{
    this->_fd = fd;
}

std::string &Source::get_path()
{
    return this->_path;
}

bool Source::operator<(const Source &ref) const {
    if (this->_fd < ref._fd) {
        return true;
    } else {
        return false;
    }
}

#include "Source.hpp"

int &Source::get_fd() {
    return this->_fd;
}

void Source::set_path(std::string path)
{
    this->_path = path;
}

std::string Source::get_path()
{
    return this->_path;
}
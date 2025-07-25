#include "Source.hpp"

Source::Source() {
    //std::cout << "Source gets created" << std::endl;
    this->_fd = -1;
    this->_path = "";
}

Source::Source(const Source &ref) {
    //std::cout << "Source copy constructor called" << std::endl;
    this->_fd = ref._fd;
    this->_path = ref._path;
}

Source::~Source() {
    //std::cout << "Source destructor called" << std::endl;
}

Source &Source::operator=(const Source &ref) {
    //std::cout << "Source copy assignment operator called" << std::endl;
    if (this != &ref) {
        this->_fd = ref._fd;
        this->_path = ref._path;
    }
    return *this;
}

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

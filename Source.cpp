#include "Source.hpp"

Source::Source() {
    //std::cout << "Source gets created" << std::endl;
    this->_fd = -1;
    this->_path = "";
    this->_pid = 0;
	this->_cgi_finished = 0;
}

Source::Source(const Source &ref) {
    //std::cout << "Source copy constructor called" << std::endl;
    this->_fd = ref._fd;
    this->_path = ref._path;
    this->_pid = ref._pid;
	this->_cgi_finished = 0;
}

Source::~Source() {
    //std::cout << "Source destructor called" << std::endl;
}

Source &Source::operator=(const Source &ref) {
    //std::cout << "Source copy assignment operator called" << std::endl;
    if (this != &ref) {
        this->_fd = ref._fd;
        this->_path = ref._path;
        this->_pid = ref._pid;
		this->_cgi_finished = 0;
    }
    return *this;
}

const int &Source::get_fd() const {
    return this->_fd;
}

std::string &Source::get_path() {
    return this->_path;
}

int &Source::get_pid() {
    return this->_pid;
}

bool &Source::get_cgi_finished() {
	return this->_cgi_finished;
}


void    Source::set_path(std::string path) {
    this->_path = path;
}

void    Source::set_fd(int fd) {
    this->_fd = fd;
}

void    Source::set_pid(int pid) {
    this->_pid = pid;
}

void 	Source::set_cgi_finished(bool status)
{
	this->_cgi_finished = status;
}

bool Source::operator<(const Source &ref) const {
    if (this->_fd < ref._fd) {
        return true;
    } else {
        return false;
    }
}

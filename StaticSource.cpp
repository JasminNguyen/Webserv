#include "StaticSource.hpp"

StaticSource::StaticSource() {
    this->_fd_out = -1;
}

StaticSource::StaticSource(const StaticSource &ref) {
    this->_fd_out = ref._fd_out;
}

StaticSource::~StaticSource() {}

StaticSource &StaticSource::operator=(const StaticSource &ref) {
    if (this != &ref) {
        this->_fd_out = ref._fd_out;
    }
    return *this;
}

int &StaticSource::get_fd_out() {
    return this->_fd_out;
}

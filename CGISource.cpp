#include "CGISource.hpp"

CGISource::CGISource() : Source() {}

CGISource::CGISource(const CGISource &ref) {
    this->_fd = ref._fd;
}

CGISource::~CGISource() {}

CGISource   &CGISource::operator=(const CGISource &ref) {
    if (this != &ref) {
        this->_fd = ref._fd;
    }
    return *this;
}

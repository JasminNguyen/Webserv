
#ifndef SOURCE_HPP
#define SOURCE_HPP
#include "webserv.hpp"

class Source {

    public:

        int &get_fd();

    private:

        int _fd;
        std::string _path;

};


#endif

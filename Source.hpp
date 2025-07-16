
#ifndef SOURCE_HPP
#define SOURCE_HPP
#include "webserv.hpp"

class Source {

    public:

        int &get_fd();

        void set_fd(int fd);
        void set_path(std::string path);
        std::string get_path();

    private:

        int _fd;
        std::string _path;

};


#endif

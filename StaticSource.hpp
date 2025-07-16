#ifndef STATIC_SOURCE_HPP
# define STATIC_SOURCE_HPP

class StaticSource {

    public:

        int &get_fd_out();

    private:

        int    _fd_out;
};

#endif

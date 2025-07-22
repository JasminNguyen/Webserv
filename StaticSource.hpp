#ifndef STATIC_SOURCE_HPP
# define STATIC_SOURCE_HPP

class StaticSource {

    public:

        StaticSource();
        StaticSource(const StaticSource &ref);
        ~StaticSource();

        StaticSource    &operator=(const StaticSource &ref);

        int             &get_fd_out();

    private:

        int             _fd_out;
};

#endif

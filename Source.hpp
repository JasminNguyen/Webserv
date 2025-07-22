
#ifndef SOURCE_HPP
# define SOURCE_HPP

# include "webserv.hpp"

class Source {

    public:

        Source();
        Source(const Source &ref);
        ~Source();

        Source      &operator=(const Source &ref);

        const int   &get_fd() const;

        void        set_fd(int fd);
        void        set_path(std::string path);
        std::string &get_path();
        bool        operator<(const Source &ref) const;

    private:

        int         _fd;
        std::string _path;

};


#endif

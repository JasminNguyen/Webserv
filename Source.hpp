#ifndef SOURCE_HPP
# define SOURCE_HPP

# include "webserv.hpp"

class Source {

    public:

        Source();
        Source(const Source &ref);
        ~Source();

        Source      &operator=(const Source &ref);

        bool        operator<(const Source &ref) const;

        const int   &get_fd() const;
        std::string &get_path();
        int         &get_pid();
		bool		&get_cgi_finished();

        void        set_fd(int fd);
        void        set_path(std::string path);
        void        set_pid(int pid);
		void		set_cgi_finished(bool status);

    protected:

        int         _fd;
        std::string _path;
        int         _pid;
		bool		_cgi_finished;
};


#endif

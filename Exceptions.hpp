#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include "webserv.hpp"

class Exceptions: public std::exception
{
    private:
        std::string _message;
    public:
        Exceptions(const std::string& message);
        virtual ~Exceptions() throw();
        virtual const char *what() const throw();
};

#endif

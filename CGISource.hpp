#ifndef CGI_SOURCE_HPP
# define CGI_SOURCE_HPP

# include "Source.hpp"

class CGISource : public Source {

    public:

        CGISource();
        CGISource(const CGISource &ref);
        ~CGISource();

        CGISource   &operator=(const CGISource &ref);

    private:

};

#endif

#include "webserv.hpp"
#include "Exceptions.hpp"


std::string trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r;");
    if (start == std::string::npos)
        return ""; // string is all whitespace

    size_t end = str.find_last_not_of(" \t\n\r;");
    return str.substr(start, end - start + 1);
}

std::string generate_date() {

    time_t now = time(NULL);          // current time (seconds since epoch)
    struct tm gmt;                    // broken-down GMT time

    // Convert to GMT/UTC
    gmt = *gmtime(&now);

    // Format buffer
    char buf[128];
    // RFC1123: "Day, DD Mon YYYY HH:MM:SS GMT"
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt);

    return std::string(buf);
}

void	unblock_fd(int fd) {
	int	flags_before;
	int flags;
	int flags_after;

	flags_before = fcntl(fd, F_GETFL);
	if (flags_before == -1) {
		// INTERNAL ERROR
		throw Exceptions("fcntl failed on getting flags before.");
		// conn.generate_error_page(webserver, "403", server_block);
		// if (conn.get_source().get_fd() != -1)
        // {
        //     return;
        // }
		// conn.generate_headers();
		// conn.get_response().assemble();
		// webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		// return;
	}

	flags = flags_before + O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) == -1) {
		// INTERNAL ERROR
		throw Exceptions("fcntl failed on setting flags.");
		// conn.generate_error_page(webserver, "403", server_block);
		// if (conn.get_source().get_fd() != -1)
        // {
        //     return;
        // }
		// conn.generate_headers();
		// conn.get_response().assemble();
		// webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		// return;
	}

	flags_after = fcntl(fd, F_GETFL);
	if (flags_after == -1) {
		// INTERNAL ERROR
		throw Exceptions("fcntl failed on getting flags after.");
		// conn.generate_error_page(webserver, "403", server_block);
		// if (conn.get_source().get_fd() != -1)
        // {
        //     return;
        // }
		// conn.generate_headers();
		// conn.get_response().assemble();
		// webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		// return;
	}
	if (flags_after - flags_before != 2048) {
		//std::cout << "Flags difference is: " << flags_after - flags_before << std::endl;
		std::cout << "Flags problem!" << std::endl;
	}
}

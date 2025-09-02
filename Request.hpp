#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <sstream>
# include <map>

class Request {

	public:

		Request();
		Request(std::string raw);
		Request(const Request &ref);
		~Request();

		Request								&operator=(const Request &ref);

		std::string							&get_raw();
		std::string							&get_method();
		std::string							&get_target();
		std::string							&get_http_version();
		std::map<std::string, std::string>	&get_headers();
		std::string							&get_body();

		void								process(int sock_fd);
		void								parse();

	private:

		std::string							_raw; // raw string
		std::string							_method; // GET, POST, DELETE
		std::string							_target; // file path
		std::string							_http_version; // default HTTP/1.1
		std::map<std::string, std::string>	_headers; // key value pairs
		std::string							_body;


		void								_parse_start_line(std::string line);
		void								_parse_header_line(std::string line);

};

#endif

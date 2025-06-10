#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <sstream>
# include <map>

class Request {

	public:

		Request(std::string raw);
		~Request();

		void	parse();

	private:

		std::string	_raw; // raw string
		std::string	_method; // GET, POST, DELETE
		std::string	_target; // file path
		std::string	_http_version; // default HTTP/1.1

		std::map<std::string, std::string>	_headers; // key value pairs


		void	_parse_start_line(std::string line);
		void	_parse_header_line(std::string line);

};

#endif

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <map>
# include <fstream>

class Response {

	public:

		Response();
		Response(const Response &ref);
		~Response();

		Response							&operator=(const Response &ref);

		std::string							&get_raw();
		std::string							&get_http_version();
		std::string							&get_status_code();
		std::string							&get_status_string();
		std::map<std::string, std::string>	&get_headers();
		std::string							&get_body();

		void								set_raw(std::string s);
		void								set_status_code(std::string s);
		void								set_status_string(std::string s);
		void								set_body(std::string s);
		void								set_header(std::string key, std::string value);

		void								assemble();

	private:

		std::string							_raw;
		std::string							_http_version;
		std::string							_status_code;
		std::string							_status_string;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

		void								_add_status_line();
		void								_add_headers();
		void								_add_static_body();
};

#endif

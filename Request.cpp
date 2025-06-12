#include "Request.hpp"

Request::Request(std::string raw) : _raw(raw) {}
Request::~Request() {}

void	Request::_parse_start_line(std::string line) {

	std::istringstream ss(line);

	ss >> this->_method;
	ss >> this->_target;
	ss >> this->_http_version;
}

void	Request::_parse_header_line(std::string line) {
	std::string	key;
	std::string	value;

	std::istringstream ss(line);

	ss >> key;
	key.erase(key.length() - 1);

	ss >> value;

	this->_headers[key] = value;
}

 void	Request::parse() {

	std::string	line;
	std::istringstream ss(this->_raw);

	std::getline(ss, line);
	_parse_start_line(line);

	while (std::getline(ss, line)) {
		if (!line.empty()) {
			_parse_header_line(line);
		} else {
			break;
		}
	}

	// PARSING THE BODY IS STILL MISSING

	/* std::cout << "Method: " << this->_method << std::endl;
	std::cout << "Target: " << this->_target << std::endl;
	std::cout << "HTTP Version: " << this->_http_version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin();
		it != this->_headers.end(); it++) {
		std::cout << "Key: " << it->first << std::endl;
		std::cout << "Value: " << it->second << std::endl;
	}
	std::getline(ss, line);
	std::cout << "TEST: " << line << std::endl; */
 }

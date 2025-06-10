#include "Request.hpp"

Request::Request(std::string raw) : _raw(raw) {}
Request::~Request() {}

void	Request::_parse_start_line(std::string line) {

	std::string::iterator it = line.begin();

	while (it != line.end() && *it != ' ') {
		this->_method += *it;
		it++;
	}
	it++;

	while (it != line.end() && *it != ' ') {
		this->_target += *it;
		it++;
	}
	it++;

	while (it != line.end()) {
		this->_http_version += *it;
		it++;
	}
}

void	Request::_parse_header_line(std::string line) {
	std::string	key;
	std::string	value;

	std::string::iterator it = line.begin();

	while (it != line.end() && *it != ' ') {
		key += *it;
		it++;
	}
	key.erase(key.length() - 1);
	it++;
	while (it != line.end()) {
		value += *it;
		it++;
	}
	this->_headers[key] = value;
}

 void	Request::parse() {

	std::string	line;
	std::istringstream ss(this->_raw);

	std::getline(ss, line);
	_parse_start_line(line);

	while (std::getline(ss, line)) {
		if (line.length() != 0) {
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

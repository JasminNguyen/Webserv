#include "Request.hpp"

Request::Request() {
	this->_body = "";
	this->_http_version = "";
	this->_method = "";
	this->_raw = "";
	this->_target = "";
}

Request::Request(std::string raw) : _raw(raw) {
	this->_body = "";
	this->_http_version = "";
	this->_method = "";
	this->_target = "";
}

Request::Request(const Request &ref) {
	this->_body = ref._body;
	this->_headers = ref._headers;
	this->_http_version = ref._http_version;
	this->_method = ref._method;
	this->_raw = ref._raw;
	this->_target = ref._target;
}

Request::~Request() {}

Request	&Request::operator=(const Request &ref) {
	if (this != &ref) {
		this->_body = ref._body;
		this->_headers = ref._headers;
		this->_http_version = ref._http_version;
		this->_method = ref._method;
		this->_raw = ref._raw;
		this->_target = ref._target;
	}
	return *this;
}

std::string	&Request::get_raw() {
	return this->_raw;
}

std::string	&Request::get_method() {
	return this->_method;
}

std::string	&Request::get_target() {
	return this->_target;
}

std::string	&Request::get_http_version() {
	return this->_http_version;
}

std::map<std::string, std::string>	&Request::get_headers() {
	return this->_headers;
}

std::string	&Request::get_body() {
	return this->_body;
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
	if (key.empty()) {
		return;
	}
	key.erase(key.length() - 1, 1);

	ss >> value;

	this->_headers[key] = value;
}

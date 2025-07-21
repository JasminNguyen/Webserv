#include "Response.hpp"

Response::Response() {
	this->_raw = "";
}

Response::Response(const Response &ref) {
	this->_body = ref._body;
	this->_headers = ref._headers;
	this->_http_version = ref._http_version;
	this->_raw = ref._raw;
	this->_status_code = ref._status_code;
	this->_status_string = ref._status_string;
}

Response::~Response() {}

Response	&Response::operator=(const Response &ref) {
	if (this != &ref) {
		this->_body = ref._body;
		this->_headers = ref._headers;
		this->_http_version = ref._http_version;
		this->_raw = ref._raw;
		this->_status_code = ref._status_code;
		this->_status_string = ref._status_string;
	}
	return *this;
}

std::string	&Response::get_raw() {
	return this->_raw;
}

std::string	&Response::get_http_version() {
	return this->_http_version;
}

std::string	&Response::get_status_code() {
	return this->_status_code;
}

std::string	&Response::get_status_string() {
	return this->_status_string;
}

std::map<std::string, std::string>	&Response::get_headers() {
	return this->_headers;
}

std::string	&Response::get_body() {
	return this->_body;
}

void	Response::assemble() {

	this->_add_status_line();
	this->_add_headers();
	this->_add_static_body();

	// How is dynamic body added? if else statement and adding functions missing

}

void	Response::_add_static_body() {
	/* std::string		line;
	std::ifstream	inFileStream;

	inFileStream.open(this->_source.c_str());
	if (inFileStream.fail()) {
		std::cerr << "Error opening source file." <<std::endl;
		throw(std::exception());
	}

	while (std::getline(inFileStream, line)) {
		this->_raw.append(line);
		this->_raw.append("\n");
	}
	inFileStream.close();
	*/
	this->_raw.append(this->_body);
}

void	Response::_add_status_line() {
	this->_raw.append("HTTP/1.1 ");
	this->_raw.append(this->_status_code);
	this->_raw.append(" ");
	this->_raw.append(this->_status_string);
	this->_raw.append("\n");
}

void	Response::_add_headers() {
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin();
		it != this->_headers.end(); it++) {
			this->_raw.append(it->first);
			this->_raw.append(": ");
			this->_raw.append(it->second);
			this->_raw.append("\n");
		}
	this->_raw.append("\n");
}

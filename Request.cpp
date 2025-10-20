#include "Request.hpp"
#include "webserv.hpp"

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



bool Request::header_val_contains_chunked(std::string &header_val)
{
	std::string val = header_val;
	for(size_t i = 0; i < val.size(); i++) //everything lowercase
	{
		if(val[i] >= 'A' && val[i] <= 'Z')
		{
			val[i] = ((char)val[i] + 32);
		}

	}

	// Split by commas
    std::vector<std::string> tokens;
    std::stringstream ss(val);
    std::string token;
    while (std::getline(ss, token, ','))
	{
        tokens.push_back(trim(token));
    }

	for(size_t i = 0; i < tokens.size(); i++)
	{
		if(tokens[i] == "chunked")
		{
			return true;
		}
	}
	return false;
}
int Request::is_chunked()
{
	for(std::map<std::string,std::string>::iterator it = this->get_headers().begin(); it != this->get_headers().end(); it++)
	{
		if(it->first == "Transfer-Encoding" || it->first == "transfer-encoding" || it->first == "TRANSFER-ENCODING")
		{
			std::string header_val = it->second;
			if(header_val_contains_chunked(header_val) == true)
			{
				return 1;
			}
		}
	}
	return 0;
}

int	Request::process(int sock_fd) {
	char buf[BUF_SIZE];
	// read into this->_req->_raw
	std::cout << "handle request on client socket" << std::endl;
	size_t n = 0;
	n = read(sock_fd, buf, sizeof(buf));
	std::cout << std::endl;
	std::cout << "buf: " << buf << std::endl;
	std::cout << std::endl;
	/* while ((n = read(sock_fd, buf, sizeof(buf))) && n == 1024) {
		this->_raw.append(buf);
	} */
	if (n < 0) {
		return -1;
	} else if (n < BUF_SIZE) {
		std::cout << "We are parsing the request" << std::endl;
		std::cout << std::endl << this->_raw << std::endl;
		this->parse();
		return 1;
	} else { // difference between n == 0 and n > 0 ???
		this->_raw.append(buf, n);
		return 0;
	}
}

void	Request::set_body(std::string tmp)
{
	this->_body = tmp;
}

void	Request::parse() {

	std::string	line;
	std::istringstream ss(this->_raw);

	std::getline(ss, line);
	_parse_start_line(line);

	// while (std::getline(ss, line)) {
	// 	if (!line.empty()) {
	// 		_parse_header_line(line);
	// 	} else {
	// 		break;
	// 	}
	// }
	//MODIFIED BY JASMIN -> let Marc review this
	while (std::getline(ss, line)) {
		if (!line.empty() && line != "\r") {
			if (line[line.size() - 1] == '\r')
			{
				line.erase(line.size()-1);
			}
			_parse_header_line(line);
		} else {
			break;
		}
	}
	if (this->_method == "POST") {
		std::string tmp;
		std::getline(ss, tmp, '\0');
		if(this->is_chunked())
		{
			this->get_body().append(tmp);
		}
		else
		{
			this->set_body(tmp);
		}
	} else {
		this->_body = "";
	}
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

	std::getline(ss, value);
	value = trim(value);
	this->_headers[key] = value;
}

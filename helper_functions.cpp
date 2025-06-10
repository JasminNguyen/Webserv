#include "webserv.hpp"


std::string trim(const std::string& str) 
{
    size_t start = str.find_first_not_of(" \t\n\r;");
    if (start == std::string::npos)
        return ""; // string is all whitespace

    size_t end = str.find_last_not_of(" \t\n\r;");
    return str.substr(start, end - start + 1);
}
#include "webserv.hpp"


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

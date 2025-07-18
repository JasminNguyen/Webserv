#include "webserv.hpp"
#include "Webserver.hpp"

int main(int argc, char **argv) {
    const char *config_file;
    std::string default_file = "./config/default.conf";

    if (argc == 2) {
        config_file = argv[1];
    } else if (argc == 1) {
        config_file = default_file.c_str();
    } else {
        std::cerr << "Error: Invalid number of arguments!" << std::endl;
        return 1;
    }

    Webserver webserv;

    try {
        webserv.parse_config(config_file);
        webserv.populate();
        webserv.launch();
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

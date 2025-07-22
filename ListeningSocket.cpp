#include "ListeningSocket.hpp"
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

ListeningSocket::ListeningSocket() {
    this->_fd = -1;
    this->_type = "Listening Socket";
}

ListeningSocket::ListeningSocket(int port, std::string host) : Socket(-1) {
    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());

    this->_type = "Listening Socket";

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "socket() failed" << std::endl;
    } else {
        this->_fd = sock_fd;
        //std::cout << "Sock fd: " << this->_fd << std::endl;
        int connection = bind(this->_fd, (struct sockaddr *)&addr, sizeof(addr));
        if (connection < 0) {
            std::cerr << "bind() failed" << std::endl;
        } else {
            int listening = listen(this->_fd, SOMAXCONN);
            if (listening < 0) {
                std::cerr << "listen() failed" << std::endl;
            }
        }

    }
}

ListeningSocket::ListeningSocket(const ListeningSocket &ref) {
    this->_fd = ref._fd;
    this->_type = ref._type;
}
ListeningSocket::~ListeningSocket() {}

ListeningSocket	&ListeningSocket::operator=(const ListeningSocket &ref) {
    if (this != &ref) {
        this->_fd = ref._fd;
        this->_type = ref._type;
    }
    return *this;
}

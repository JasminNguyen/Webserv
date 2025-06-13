
#include "webserv.hpp"
#include "config_parser.hpp"

int start_miniserver()
{   
    //create socket
    struct sockaddr_in address;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("cannot create socket"); 
        return -1; 
    }

    //make address reusable
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }

    const int PORT = 8090;
    memset(&address, 0, sizeof(address)); 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons(PORT); 

    //bind address to an ip and port
    if (bind(server_fd,(struct sockaddr *)&address,sizeof(address)) < 0) 
    { 
        perror("cannot bind"); 
        return 0; 
    }
    //listen for incoming connection
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return -1;
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK); // non-blocking server socket

    //create vector for multiple sockets
    std::vector<struct pollfd> pollfds; 
    pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    pollfds.push_back(server_pollfd);

    while (1) 
    {
        int n = poll(pollfds.data(), pollfds.size(), -1);
        if (n < 0) {
            perror("poll failed");
            break;
        }

        unsigned int i = 0;
        while (i < pollfds.size()) 
        {
            if (pollfds[i].revents & POLLIN) 
            {
                if(pollfds[i].fd == server_fd) //check that it's the listening socket
                {
                     // Accept new client
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd >= 0) 
                    {
                        fcntl(client_fd, F_SETFL, O_NONBLOCK); // make client non-blocking
                        pollfd client;
                        client.fd = client_fd;
                        client.events = POLLIN;
                        pollfds.push_back(client);
                        std::cout << "New client connected\n";
                    }
               
                }
                else
                {
                    // Handle client data
                    if(pollfds[i].fd != server_fd) //make sure it's not the listening socket
                    {
                        char buf[1024] = {0};
                        int bytes = read(pollfds[i].fd, buf, sizeof(buf));
                        std::cout << "bytes is: " << bytes << std::endl; 
                        if (bytes <= 0) //if nothing to read
                        {
                            close(pollfds[i].fd);
                            pollfds.erase(pollfds.begin() + i);
                            std::cout << "Client disconnected" << std::endl;
                            continue; // don't increment i we just go to the next socket because here there is nothing to read
                        } 
                        else //if something to read
                        {
                            std::cout << "Client said: " << buf << "\n";
                            const char *response =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 18\r\n"
                                "\r\n"
                                "Hello from server!";
                            write(pollfds[i].fd, response, strlen(response));
                        }
                    }
                   
                }   
                
            }
            i++;
        }
    }

    return 0;

}

int main(int argc, char *argv[])
{
    if(argc != 2) //to be implemented: default path 
    {
        std::cout << "Error: Invalid number of arguments!" << std::endl;
        return -1;
    }
    //start_miniserver();
   
    configParser configParser;
    std::vector<std::string> tokenVector;

    tokenVector = configParser.tokenize(argv[1]);
    for(size_t i = 0; i < tokenVector.size(); i++)
    {
        std::cout << tokenVector[i] << std::endl;
    }
    configParser.parse_server_block(tokenVector);
    
    for(size_t i = 0; i < configParser.serverConfigVector.size(); i++)
    {
        std::cout << "element [" << i << "]: " << configParser.serverConfigVector[i] << std::endl;
    }

    

    





    // if(configParser::config_parser(argv[1], serverConfigVector) == -1)
    // {
    //     perror("config file");
    //     return -1;
    // }
    
    // for(size_t i = 0; i < serverConfigVector.size(); i++)
    // {
    //     std::cout << "element [" << i << "]: " << serverConfigVector[i] << std::endl;
    // } 
    // return 0;
}



   
//     /* NOTES
//     The accept system call grabs the first connection request on the queue of pending 
//     connections (set up in listen) and creates a new socket for that connection.
    
//     The original socket that was set up for listening is used only for accepting connections, 
//     not for exchanging data. By default, socket operations are synchronous, 
//     or blocking, and accept will block until a connection is present on the queue.*/


//     // 5. send and receive messages
//     /*read is a blocking function so we will stay in this function until we have an input from the client to continue the process, 
//     we don't want to wait so we use poll to test fd to test if a fd is ready for operation*/

// differenciate listening socket and client socket
//the first time it's about the listening socket which has nothing to read from 
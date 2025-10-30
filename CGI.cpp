///assuming that we have a .py extention

#include "webserv.hpp"
#include "CGI.hpp"
#include "Connection.hpp"
#include "Request.hpp"
#include "Webserver.hpp"
#include "Source.hpp"
#include "Exceptions.hpp"



char ** CGI::construct_envp(Request& request, configParser::ServerConfig & server_block)
{
    //which envp to pass?
    //probably depends on the method as well
    /*REQUEST_METHOD	Tells the script whether it's GET, POST, etc.
    SCRIPT_NAME	The path of the script relative to the root
    QUERY_STRING	Needed for GET requests to get input data
    CONTENT_LENGTH	Required for POST; tells how many bytes to read
    CONTENT_TYPE	Describes the body data type (e.g. application/x-www-form-urlencoded)
    SERVER_PROTOCOL	Usually HTTP/1.1 — CGI uses this for responses
    SERVER_NAME	The host part of the URL
    SERVER_PORT	The port the server is listening on
    //REMOTE_ADDR	IP address of the client
    GATEWAY_INTERFACE	Always "CGI/1.1"*/

    std::vector<std::string> env;
    env.push_back("REQUEST_METHOD=" + request.get_method());
    // "/cgi-script/foo.py" or "/cgi-bin/foo.py?name=Jasmin" in a GET request (query string present)
    if(request.get_target().find("?")!= std::string::npos)//if ? found in uri -> "/cgi-bin/foo.py?name=Jasmin"
    {
        int pos_question_mark = request.get_target().find("?");
        std::string script_name = request.get_target().substr(0, pos_question_mark);
        env.push_back("SCRIPT_NAME=" + script_name);
    }
    else // -> "/cgi-bin/foo.py"
    {
        env.push_back("SCRIPT_NAME=" + request.get_target());
    }
    //query string
    std::string query_string = "";
    if(request.get_target().find("?") != std::string::npos)
    {
        int pos_question_mark = request.get_target().find("?");
        query_string = request.get_target().substr(pos_question_mark + 1);
    }
    env.push_back("QUERY_STRING=" + query_string);


    //find content lenght in header map
    std::map<std::string, std::string>& headers = request.get_headers();

    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
    {
        if (it->first == "Content-Length")
        {
            //std::cout << "The cgi content length: " << it->second << std::endl;
            env.push_back("CONTENT_LENGTH=" + it->second);
        }
    }
    //find content type in header map
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
    {
        if (it->first == "Content-Type")
        {
            //std::cout << "The cgi content type is: " << it->second << std::endl;
            env.push_back("CONTENT_TYPE=" + it->second);
        }
    }
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
   //find content type in header map
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
    {
        if (it->first == "Host")
        {
            env.push_back("SERVER_NAME=" + it->second);
        }
    }
	std::stringstream ss;
	ss << server_block.port;
    env.push_back("SERVER_PORT=" + ss.str());
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");

    int array_length = env.size();
	char **envp = (char**)malloc((array_length + 1) * sizeof(char*));
    for(int i = 0; i < array_length; i++)
    {
        envp[i] = strdup(env[i].c_str());
    }
    envp[array_length] = NULL;

    return envp;
}



char** CGI::construct_argv(const char* &script_path,Request &request)
{
    char *converted_script_path = strdup(script_path); //don't forget to free strdup allocates mem
    char **argv = (char**)malloc(3 * sizeof(char*));
    //argv[0] = (char*)"/usr/bin/python3"; // argv[0] = interpreter (I assume that we just use a python script to run cgi)
    if(request.get_target().substr(request.get_target().size() - 3).compare(".py") == 0)
    {
		std::string path = "/usr/bin/python3";
		argv[0] = strdup(path.c_str());
    }
    else if(request.get_target().substr(request.get_target().size() - 4).compare(".php") == 0)
    {
		std::string path = "/usr/bin/php";
		argv[0] = strdup(path.c_str());
    }
    else if(request.get_target().substr(request.get_target().size() - 3).compare(".sh") == 0)
    {
		std::string path = "/usr/bin/sh";
		argv[0] = strdup(path.c_str());
    }
    argv[1] = converted_script_path;  // argv[1] = script_path that I constructed earlier
    argv[2] = NULL;   // end of array

    return argv;
}

/*will probably remove this and replace it with the match_location_block()*/
// std::string CGI::construct_script_path(Request& request, configParser::ServerConfig & server_block)
// {
//     std::string target_uri = request.get_target(); //  something like this "/cgi-bin/foo.py?querypahtk"
//     std::string script_path;
//     bool match_found = false;

//     // Start looking for the second slash *after* the first one
//     std::size_t first = target_uri.find('/');
//     std::size_t second = target_uri.find('/', first + 1);
//     std::size_t pos_question_mark = target_uri.find('?');
//     std::string target_to_append;
//     std::string target_to_match = target_uri.substr(0, second); // make "/cgi-bin" out of it
//     if(pos_question_mark != std::string::npos)
//     {
//         target_to_append = target_uri.substr(second, pos_question_mark);// "/foo.py"
//     }
//     target_to_append = target_uri.substr(second, target_uri.size()); // make "/foo.py" out of it -> to append later

//     //iterating through all of the locations structs in the Locations vector to find the right path (that is "/cgi-bin" in this case)
//     for(size_t i = 0; i < server_block.locations.size(); i++)
//     {
//         if(server_block.locations[i].path == target_to_match) //found a match in one of the location blocks
//         {
//             script_path = server_block.locations[i].root + target_to_append; // result: "/Users/jasminn/webserv/cgi-scripts/foo.py"
//             match_found = true;
//             break;
//         }
//     }
//     if(match_found == false)
//     {
//         std::cerr << "No match found in the location blocks" << std::endl; //throw error!!!!!
//     }

//     return script_path;

// }

void CGI::run_cgi(Request& request, configParser::ServerConfig & server_block, Webserver & webserver, Connection &conn)
{
	//std::cout << "file is:" << conn.get_source().get_path().c_str() << std::endl;
	//conn.get_source().set_path("/cgi/post_foto.py");
    if (access(conn.get_source().get_path().c_str() , R_OK) == -1)
    {
        std::cerr << "File doesn't exist or isn't readable. - CGI" << std::endl;
        if (errno == EACCES)
        {
            conn.generate_error_page(webserver, "403", server_block);
        }
        else if (errno == ENOENT)
        {
            conn.generate_error_page(webserver, "404", server_block);
        }
        else
        {
            conn.generate_error_page(webserver, "500", server_block);
        }
        if (conn.get_source().get_fd() != -1)
        {
            return; // body will be read later
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;

    }
    if (access(conn.get_source().get_path().c_str() , X_OK) == -1)
    {
        if (errno == EACCES)
        {
            conn.generate_error_page(webserver, "403", server_block);
            if (conn.get_source().get_fd() != -1)
            {
                return;
            }
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
        }
    }
    //creating 2 pipes (one that the cgi reads and one that the cgi writes to)
    /*
    sooo we have 4 filedescriptors:

    in_pipe[1]	write end	your server writes the request body here
    in_pipe[0]	read end	the CGI script reads it from stdin
    out_pipe[1]	write end	the CGI writes its output to this
    out_pipe[0]	read end	your server reads the CGI output here*/
    int in_pipe[2];
    int out_pipe[2];
    if(pipe(in_pipe)== -1 || pipe(out_pipe) == -1)
    {
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
    }
    pid_t pid = fork();
	// std::cout << "pid is: " << pid << std::endl;
    if(pid == -1)
    {
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
    }
    conn.get_source().set_pid(pid);
    if(pid == 0) //child
    {
        //redirecting the pipes ends used by cgi
        dup2(in_pipe[0], STDIN_FILENO); //cgi reads from here
        dup2(out_pipe[1], STDOUT_FILENO); //cgi writes here

        dup2(out_pipe[1], STDERR_FILENO); // send CGI stderr to the same pipe

        //closing unused pipe ends (those that used by server)
        if (close(in_pipe[1]) == -1) {
			// INTERNAL
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
			    return;
			}
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
		}
        if (close(out_pipe[0]) == -1) {
			// INTERNAL
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
			    return;
			}
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
		}

        //I have to close the other two as well to avoid leaking fds (I can do that since I duplicated them and they replace stdin/out)
        if (close(in_pipe[0]) == -1) {
			// INTERNAL
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
			    return;
			}
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
		}
        if (close(out_pipe[1]) == -1) {
			// INTERNAL
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
			    return;
			}
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
		}

        //execute
        // set up script_path, argv + envp, then exec
        //execve("/usr/bin/php-cgi", argv, envp);
        //const char* script_path = CGI::construct_script_path(request, server_block).c_str();

        //DECIDING IF IT'S GET OR POST
        // if(/*post request*/)
        // {

        // }

        const char *script_path = conn.get_source().get_path().c_str();
      	//std::cout << "script path in run_cgi(): " << script_path << std::endl;

        char **argv = construct_argv(script_path, request);
        char **envp = construct_envp(request, server_block);

        //change directory before execve
        std::string change_dir_to;
        std::string script_path_string = strdup(script_path);
        int pos_last_slash = script_path_string.rfind("/");
        change_dir_to = script_path_string.substr(0, script_path_string.size() - (script_path_string.size() - pos_last_slash));
        //std::cout << "change dir to is: " << change_dir_to << std::endl;
		//char buf[100];

		//std::cout << "pwd: " << getcwd(buf, 100) << std::endl;

        if (chdir(change_dir_to.c_str()) == -1)
        {
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
				return;
			}
			//generate headers
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
        }
		//std::cout << "pwd: " << getcwd(buf, 100) << std::endl;

		//strip script path to get only the name of the script (since we changed the directory of the script it's all we need)
		std::string script_name = script_path_string.substr(pos_last_slash + 1);
		//std::cout << "script name is: " << script_name << std::endl;

        if(execve(script_name.c_str(), argv, envp) == -1)
		{
			conn.generate_error_page(webserver, "500", server_block);
			if (conn.get_source().get_fd() != -1)
			{
				return;
			}
			//generate headers
			conn.generate_headers();
			conn.get_response().assemble();
			webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
			return;
		}

        perror("execve failed"); // only runs if exec fails
        exit(1);
    }
    else // parent (our beautiful server)
    {


    // closing unused pipe ends
    if (close(in_pipe[0]) == -1) {
		// INTERNAL
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
	}  // we don't read from stdin pipe
    if (close(out_pipe[1]) == -1) {
		// INTERNAL
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
	} // we don't write to CGI output pipe

    // write to in_pipe[1] → CGI stdin (cgi instructions)
    // read from out_pipe[0] ← CGI output (result of what cgi made)
    // std::cout << "body of POST request is: " << request.get_body() << std::endl;
    // std::cout << "size of POST request is: " << request.get_body().size() << std::endl;
    if (write(in_pipe[1], request.get_body().c_str(), request.get_body().size()) == -1) {
		// INTERNAL ISSUE
		// what should happen here?
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
	} //writing request to CGI via pipe
    if (close(in_pipe[1]) == -1) {
		// INTERNAL
		conn.generate_error_page(webserver, "500", server_block);
		if (conn.get_source().get_fd() != -1)
        {
            return;
        }
		conn.generate_headers();
		conn.get_response().assemble();
		webserver.add_pollout_to_socket_events(conn.get_socket().get_fd());
		return;
	} //close that pipe

    webserver.add_connection_to_poll(out_pipe[0]); //add out_pipe end to pollfd vector
   // webserver.find_triggered_source(webserver.get_polls());

   //put out_pipe in Source-Connection map so that we know which CGI response belongs to which client

    conn.get_source().set_fd(out_pipe[0]);
    }
}


//we will probably call this run_cgi() from a Connections instance  -> handle_request() or possible from handle_source_event();
//don't forget to freeeee
//free(argv[1]);
// delete argv;

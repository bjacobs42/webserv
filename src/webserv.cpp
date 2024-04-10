#include "webserv.hpp"
#include "VHost.hpp"

int	main(int argc, char **argv)
{
	bool			running = true;
	t_server		server;
	std::vector<size_t>	connections;
	int			status;
	char			buffer[BUFF_READ_SIZE];
	t_httprequest		request;
	VHost			virtual_host;
	t_httpresponse		response;
	t_route			alt_route;

	argv = argv;
	if (argc <= 1)
		return (error("no configuration file provided", ERR_NO_CONFIG));
	add_socket(server, 0, 9999);
	add_socket(server, 0, 9998);
	add_socket(server, 0, 9997);
	add_socket(server, 0, 9996);
	add_socket(server, 0, 9995);
	virtual_host.set_err_page(HTTP_BAD_METHOD, "./res/custom_error.html");
	virtual_host.set_root("/home/jburleson/documents/codam/webserv2/static");
	virtual_host.set_autoindex(true);
	virtual_host.allow_method(HTTP_GET);
	virtual_host.allow_method(HTTP_POST);
	alt_route.index = "/index.html";
	alt_route.alias = "/alt";
	alt_route.root = "/home/jburleson/documents/codam/webserv2/res";
	alt_route.autoindex = true;
	alt_route.allowed_methods.insert(HTTP_GET);
	virtual_host.add_route(alt_route);
	while (running)
	{
		poll_server(server);
		for (size_t &idx: server.listens)
		{
			if (server.sockets[idx].revents & POLLIN)
				add_connection(server, server.sockets[idx], connections);
		}
		for (size_t &idx : connections)
		{
			if (server.sockets[idx].revents & POLLIN)
			{
				memset(buffer, 0, BUFF_READ_SIZE);
				status = read(server.sockets[idx].fd, buffer, BUFF_READ_SIZE);
				if (status == -1)
					std::cerr << " read(): " << strerror(errno) << std::endl;
				if (status == 0)
				{
					close(server.sockets[idx].fd);
					server.sockets[idx].fd = -1;
					std::cout << "a client has diconnected." << std::endl;
				}
				if (status == -1 || status == 0)
					continue ;
				request = parse_request(buffer);
				request.client = server.sockets[idx];
				response = virtual_host.process_request(request);
				send_response(response);
			}
		}
	}
	return (OK);
}


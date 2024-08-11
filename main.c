#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "http_parser.h"
#include "http_responder.h"
#include "logging.h"

#define REQ_LIMIT 8192

char *PAGES[] = {"/", "/projects", "/links", "/site"};
char *PAGESDIR = "pages";

int create_server() {
	logging_server_start();
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_sockname;

	server_sockname.sin_family = AF_INET;
	server_sockname.sin_port = htons(8080); 
	server_sockname.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind listening socket to address and port */
	if (bind(socket_fd, (struct sockaddr *)&server_sockname, sizeof(server_sockname)) == -1) {
		perror("Failed to bind socket");
		return 1;
	}
	printf("Socket binding successful to port %d at address %d\n", ntohs(server_sockname.sin_port), server_sockname.sin_addr.s_addr);
	
	/* listen on socket for attempts to connect */
	if (listen(socket_fd, 128) == -1) {
		perror("Failed to listen on socket");
		return 1;
	}
	printf("Socket is listening on port %d\n", ntohs(server_sockname.sin_port));

	return socket_fd;
}

int accept_new(int source_fd) {
	struct sockaddr_in client_sockname;
	socklen_t client_addr_len = sizeof(client_sockname);

	int client_fd = accept(source_fd, (struct sockaddr *)&client_sockname, &client_addr_len);
	if (client_fd == -1) {
		perror("Failed to accept connection on listening socket");
	}

	printf("Accepted new client socket connection!\n");
	printf("File descriptor: %d\n", client_fd);
	printf("Client address: %s\n", inet_ntoa(client_sockname.sin_addr));
	printf("Client port: %d\n\n", ntohs(client_sockname.sin_port));

	return client_fd;
}

void serve_page(int source_fd, char *path) {
	int html_fd = -1;

	if (strcmp(path, "/") == 0) {
		html_fd = open("pages/index.html", O_RDONLY);	
	} else {
		char *full_path = malloc(strlen(path) + strlen(PAGESDIR) + 6);
		sprintf(full_path, "%s%s.html", PAGESDIR, path);
		printf("Request for %s from fd: %i\n", full_path, source_fd);

		html_fd = open(full_path, O_RDONLY);	
		free(full_path);
	}

	if (html_fd == -1) {
		perror("Error opening file");
		return;
	}
	
	/* response */
	http_res_fdsend(200, source_fd, html_fd);

	printf("PAGE SERVED: now closing html page fd: %d and client fd: %d\n", html_fd, source_fd);
	if (close(source_fd) == -1)
		perror("Error closing source_fd");
	if (close(html_fd) == -1)
		perror("Error closing html_fd");
}

void handle_request(int source_fd, char* req) {
	int parse_error = 0;
	struct http_req this_req = http_parse_req(req, &parse_error);
	if (parse_error) {
		printf("Error parsing request");

		free(this_req.method);
		this_req.method = NULL;
		free(this_req.path);
		this_req.path = NULL;

		http_res_fdsend(400, source_fd, -1);   /* Bad Request */
		return;
	}

	if (strcmp(this_req.method, "GET") == 0) {
		for (int i = 0; i < sizeof(PAGES) / sizeof(PAGES[0]); i++) {
			if (strcmp(this_req.path, PAGES[i]) == 0) {
				serve_page(source_fd, this_req.path);
				return;
			}
		}
		http_res_fdsend(404, source_fd, -1);
	} else {
		http_res_fdsend(405, source_fd, -1);  /* "Method Not Allowed" */ 
	}

	free(this_req.method);
	free(this_req.path);
}

void read_from_and_respond(int source_fd) {
    char read_buffer[BUFFER];
    ssize_t bytes_read;
    char *request = NULL;
    size_t total_length = 0;

    int flags = fcntl(source_fd, F_GETFL, 0);
    fcntl(source_fd, F_SETFL, flags | O_NONBLOCK);

    while ((bytes_read = read(source_fd, read_buffer, sizeof(read_buffer) - 1)) > 0) {
		if (total_length > REQ_LIMIT) {
			bytes_read = -1;
			break;
		}
        read_buffer[bytes_read] = '\0';

        char *temp = realloc(request, total_length + bytes_read + 1);
        if (!temp) {
            perror("Failed to allocate memory for request\n");
            free(request);
            return;
        }

        request = temp;
        memcpy(request + total_length, read_buffer, bytes_read + 1);
        total_length += bytes_read;
    }

    if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("Error reading request");
        free(request);
        return;
    }

    if (request) {
        request[total_length] = '\0';
    }

    handle_request(source_fd, request);
    free(request);
}

int main() {
	int socket_fd = create_server(); /* listening socket */

	int nfds = socket_fd + 1;	/* initial value for highest value file descriptor for select() */
	fd_set rfds;				/* set of file descriptors watched for ready-to-read events */
	FD_ZERO(&rfds);
	FD_SET(socket_fd, &rfds);
	struct timeval select_timeout;

	while (1) {
		/* redefine timeval struct */ 
		select_timeout.tv_sec = 5;
		select_timeout.tv_usec = 0;
		/* copy rfds */
		fd_set temp_rfds = rfds;

		int select_return_status = select(nfds, &temp_rfds, NULL, NULL, &select_timeout);
		if (select_return_status == -1)
			perror("select() error status\n");
	
		/* iterate through the file descriptors */
		for (int i = 0; i < nfds; i++) {
			if (FD_ISSET(i, &temp_rfds)) {
				if (i == socket_fd) {	/* listening socket */
					int client_fd = accept_new(socket_fd);
					FD_SET(client_fd, &rfds);
					if (client_fd >= nfds) {
						nfds = client_fd + 1;
						printf("Updated nfds to %d\n", nfds);
					}
				} else {				/* client connection socket */
					read_from_and_respond(i);   /* closes fd after response */
					printf("Clearing fd %d from rfds\n\n", i);
					FD_CLR(i, &rfds);
				}
			}
		}
	}
	return 0;
}

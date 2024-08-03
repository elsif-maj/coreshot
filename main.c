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

#define BUFFER 4096

struct http_req {
	char *method;
	char *path;
};

int create_server() {
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
	printf("Socket is listening on port %d\n", ntohs(server_sockname.sin_port)); /* move this stuff into some logger.h or something */

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
	printf("Client port: %d\n", ntohs(client_sockname.sin_port));

	return client_fd;
}

void serve_page(int source_fd) {
	char html_buffer[BUFFER];
	char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
	ssize_t bytes_read, bytes_written;

	int html_fd = open("www/testLarge.html", O_RDONLY);	
	if (html_fd == -1) {
		perror("Error opening file");
		return;
	}

	bytes_written = write(source_fd, &headers, sizeof(headers));
	if (bytes_written == -1) {
		perror("Error writing headers");
		return;
	}

	while ((bytes_read = read(html_fd, &html_buffer, sizeof(html_buffer) - 1)) > 0) {
		bytes_written = 0;
		bytes_written = write(source_fd, &html_buffer, bytes_read);
		if (bytes_written < bytes_read) {
			perror("Couldn't write whole buffer to source_fd");
			break;
		}
	}
	
	if (close(source_fd) == -1)
		perror("Error closing source_fd");
	if (close(html_fd) == -1)
		perror("Error closing html_fd");
}

/* refactor for 'unit start' and 'unit end' pointing to first and last chars -- get rid of offset */
struct http_req parse_req(char *req, int *error) {
	printf("%s\n", req);

	int offset = 0;
	struct http_req this_req = { NULL, NULL };

	/* parse http method */
	char *method_end = req;
	/* allow only 8 bytes of reading to find HTTP method */
	while (*method_end != ' ' && offset < 8 && *method_end != '\0') {
		method_end++;
		offset += 1;
	}

	if (*method_end == ' ') {
		char *method = malloc(offset + 1);
		if (method == NULL) {
			printf("Could not allocate memory for request method");
			*error = 1;
			return this_req;
		}
		memcpy(method, req, offset);
		method[offset] = '\0';
		this_req.method = method;
	} else {
		*error = 1;
		printf("Could not parse request method");
	}

	/* parse http path */
	char *path_end = method_end + 1;
	offset = 0;
	/* allow until end of req to find HTTP path */
	while (*path_end != '\0' && *path_end != ' ')
		path_end++;

	if (*path_end == ' ') {
		char *path = malloc(path_end - method_end);   /* contains one extra byte for '\0' */
		if (path == NULL) {
			printf("Could not allocate memory for request path");
			*error = 1;
			return this_req;
		}
		memcpy(path, method_end + 1, path_end - method_end - 1);
		path[path_end - method_end - 1] = '\0';
		this_req.path = path;
	} else {
		*error = 1;
		printf("Could not parse request path");
	}
	
	return this_req;
}

void handle_request(int source_fd, char* req) {
	int parse_error = 0;
	struct http_req this_req = parse_req(req, &parse_error);
	if (parse_error) {
		printf("Error parsing request");
		/* reply_400(): handle error by sending 400 ? */
		return;
	}

	if (strcmp(this_req.method, "GET") == 0) {
		printf("Do GET things\n");

		if (strcmp(this_req.path, "/") == 0)
			printf("We've got a '/' path!\n");
	} else {
		printf("Not a GET request");
		/* send something suggesting unsupported method? */
	}

	serve_page(source_fd);
}

void read_from(int source_fd) {
	char read_buffer[BUFFER];
	ssize_t bytes_read;
	char *request = NULL; 

	
    int flags = fcntl(source_fd, F_GETFL, 0);
    fcntl(source_fd, F_SETFL, flags | O_NONBLOCK);
	/* handle maximum request size */
	while ((bytes_read = read(source_fd, read_buffer, sizeof(read_buffer) - 1)) > 0) {
		read_buffer[bytes_read] = '\0';

		char *temp = realloc(request, (request ? strlen(request) : 0) + bytes_read + 1);
		if (!temp) {
			perror("Failed to allocate memory for request");
			free(request);
			return;
		}

		request = temp;
		strcat(request, read_buffer);
	}

	/* check errno for expected and non-problematic statuses from non-blocking reads of no data */
	if (bytes_read == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("Error reading request");
			free(request);
			return;
		}
	}

	handle_request(source_fd, request);
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
			perror("select() error status");
	
		/* iterate through the file descriptors */
		for (int i = 0; i < nfds; i++) {
			if (FD_ISSET(i, &temp_rfds)) {
				if (i == socket_fd) {	/* listening socket */
					int client_fd = accept_new(socket_fd);
					FD_SET(client_fd, &rfds);
					if (client_fd >= nfds) 
						nfds = client_fd + 1;
				} else {				/* client connection socket */
					read_from(i);
					FD_CLR(i, &rfds);
				}
			}
		}
	}
	return 0;
}

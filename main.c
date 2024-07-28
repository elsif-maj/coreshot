#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);		/* listening socket */

	struct sockaddr_in server_sockname, client_sockname;
	int client_fd;

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

       
	/* int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout); */

	/* variables and datastructures for the select() polling loop below */
	int nfds = socket_fd + 1;	/* initial value for highest value file descriptor for select() */
	fd_set rfds;				/* set of file descriptors watched for ready-to-read events */
	FD_ZERO(&rfds);		
	FD_SET(socket_fd, &rfds);
	struct timeval select_timeout;

	/* What we're doing in the while loop: 
	 * - create a copy of our file descriptor set (done)
	 * - select()  (done)
	 * - handle read results:
	 *    - accept() on fd == listener
	 *    - read() otherwise
	 *		- parse & if GET, write() the html file
	 */
	while (1) {
		/* redefine timeval struct -- I believe this is needed after select() returns */
		select_timeout.tv_sec = 5;
		select_timeout.tv_usec = 0;
		/* copy rfds */
		fd_set temp_rfds = rfds;	/* really? */

		/* lfg */
		int select_return_status = select(nfds, &temp_rfds, NULL, NULL, &select_timeout);
		if (select_return_status == -1)
			perror("select() error status");
		else if (select_return_status == 0)
			printf("select() timeout");
	
		/* iterate through the file descriptors */
		for (int i = 0; i < nfds; i++) {
			/* file descriptor 'i' exists in the ready-to-read set mutated by select() at return time */
			if (FD_ISSET(i, &temp_rfds)) {
				if (i == socket_fd) {	/* listening socket */
					printf("hello listener\n");
				} else {				/* client connection socket */
					printf("hello client\n");
				}
			}
		}


		/* old stuff */
		socklen_t client_addr_len = sizeof(client_sockname);
		client_fd = accept(socket_fd, (struct sockaddr *)&client_sockname, &client_addr_len);
		if (client_fd == -1) {
			perror("Failed to accept connection on listening socket");
		}

		printf("Accepted new client socket connection!\n");
		printf("File descriptor: %d\n", client_fd);
		printf("Client address: %s\n", inet_ntoa(client_sockname.sin_addr));
		printf("Client port: %d\n", ntohs(client_sockname.sin_port));
	}

	return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sockname;

	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(8080); 
	sockname.sin_addr.s_addr = htonl(INADDR_ANY); 

	if (bind(socket_fd, (struct sockaddr *)&sockname, sizeof(sockname)) == -1) {
		perror("Failed to bind socket");
		return 1;
	}
	
	if (listen(socket_fd, 5) == -1) {
		perror("Failed to listen on socket");
		return 1;
	}

	printf("Socket binding successful to port %d at address %d\n", ntohs(sockname.sin_port), sockname.sin_addr.s_addr);

	// is there a better way to test stuff like this instead of a busyloop?
	while (1) {

	}

	// what's up with Telnet?

	return 0;
}

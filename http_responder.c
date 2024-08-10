#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "http_responder.h"

struct http_status_msgs {
	int status;
	char *msg;
};

/* create array of structs and accessor method for getting http status messages */

static char* generate_headers(int status) {
	/* make this sensitive to headers size calculation */
	char *headers = malloc(512);
	if (headers == NULL)
		return NULL;
	sprintf(headers, "HTTP/1.1 %d OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n", status);	
	return headers;
}

void http_res_fdsend(int status, int client_fd, int body_fd) {
	/* Headers */
	char *headers = generate_headers(status);
	if (headers == NULL) {
		perror("Error allocatinhg memory for headers");
		return;
	}

	ssize_t bytes_read, bytes_written;
	int html_fd;

	bytes_written = write(client_fd, headers, strlen(headers));
	if (bytes_written == -1) {
		perror("Error writing headers");
		return;
	}
	free(headers);

	/* Body */
	if (body_fd != -1) {
		char html_buffer[BUFFER];
		while ((bytes_read = read(body_fd, &html_buffer, sizeof(html_buffer) - 1)) > 0) {
			bytes_written = 0;
			bytes_written = write(client_fd, &html_buffer, bytes_read);
			if (bytes_written < bytes_read) {
				perror("Couldn't write whole buffer to source_fd");
				break;
			}
		}
	}
}

/*  stub for non-file-based response bodies
 *void http_res_ssend(int status, char *body) {
 *
 *}
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "http_responder.h"

struct http_status_msg {
	int status;
	char *msg;
};

struct http_status_msg http_status_msgs[] = {
	{200, "OK"},
	{400, "Bad Request"},
	{404, "Not Found"},
	{405, "Method Not Allowed"}
};

static char *get_status_msg(int status) {
	for (int i = 0; i < (sizeof(http_status_msgs) / sizeof(http_status_msgs[0])); i++) {
		if (status == http_status_msgs[i].status) {
			return http_status_msgs[i].msg;	
		}
	}
	return NULL;
}

static char* generate_headers(int status) {
	char *headers = malloc(128);
	if (headers == NULL)
		return NULL;
	char *status_msg = get_status_msg(status);
	if (status_msg == NULL)
		return NULL;

	sprintf(headers, "HTTP/1.1 %d %s\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n", status, status_msg);	
	return headers;
}

void http_res_fdsend(int status, int client_fd, int body_fd) {
	/* Headers */
	char *headers = generate_headers(status);
	if (headers == NULL) {
		perror("Error allocating memory or producing strings for headers");
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
		if (close(body_fd) == -1)
			perror("Error closing body_fd");
	} 
	if (close(client_fd) == -1)
		perror("Error closing source_fd");
}

/*  stub for non-file-based response bodies */
/*
 * void http_res_ssend(int status, char *body) {
 *
 * }
 *
 */

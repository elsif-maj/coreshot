#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_parser.h"

#define PARSE_LIMIT 8

/* parse http method */
char *http_method_parse(struct http_req *this_req, char *req, int *error) {
	int offset = 0;
	char *method_end = req;

	/* allow only 8 bytes of reading to find HTTP method */
	while (*method_end != ' ' && offset < PARSE_LIMIT && *method_end != '\0') {
		method_end++;
		offset += 1;
	}

	if (*method_end == ' ') {
		char *method = malloc(offset + 1);
		if (method == NULL) {
			printf("Could not allocate memory for request method\n");
			*error = 1;
			return method_end;
		}
		memcpy(method, req, offset);
		method[offset] = '\0';
		this_req->method = method;
		printf("THIS_REQ.METHOD is: %s\n", this_req->method);
	} else {
		*error = 1;
		printf("Could not parse request method\n");
	}
	return method_end;
}

/* parse http path */
void http_path_parse(struct http_req *this_req, char *req, char *method_end, int *error) {
	char *path_end = method_end + 1;
	/* allow until end of req to find HTTP path */
	while (*path_end != '\0' && *path_end != ' ')
		path_end++;

	if (*path_end == ' ') {
		char *path = malloc(path_end - method_end);   /* contains one extra byte for '\0' */
		if (path == NULL) {
			printf("Could not allocate memory for request path\n");
			*error = 1;
			return; 
		}
		memcpy(path, method_end + 1, path_end - method_end - 1);
		path[path_end - method_end - 1] = '\0';
		this_req->path = path;
		printf("THIS_REQ.PATH is: %s\n", this_req->path);
	} else {
		*error = 1;
		printf("Could not parse request path\n");
	}

}

/* parse path and method from http request */
struct http_req http_parse_req(char *req, int *error) {
	struct http_req this_req = { NULL, NULL };

	/* refactor for 'unit start' and 'unit end' pointing to first and last chars -- get rid of offset */
	char *method_end = http_method_parse(&this_req, req, error);
	if (*error == 1)
		return this_req;

	http_path_parse(&this_req, req, method_end, error);
	return this_req;
}

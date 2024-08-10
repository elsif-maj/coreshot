#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

struct http_req {
	char *method;
	char *path;
};

struct http_req http_parse_req(char*, int*);

#endif /* HTTP_PARSER_H */

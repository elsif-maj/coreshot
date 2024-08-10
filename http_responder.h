#ifndef HTTP_RESPONDER
#define HTTP_RESPONDER

#define BUFFER 4096

void http_res_fdsend(int status, int client_fd, int body_fd); 

#endif /* HTTP_RESPONDER */

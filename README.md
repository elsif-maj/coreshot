coreshot README
===============

coreshot is a minimalist http server that supports responding with file payloads to GET requests with the intended function of being able to serve static webpages. This is implemented to be nonblocking via asyncronous event-driven I/O using select() -- with a poll() or epoll() update on the agenda.

coreshot does not fully implement any generation of HTTP specification -- functioning instead by parsing only the initial lines of HTTP requests for method and path, and responding with appropriate content given GET requests for supported paths. Customizing the values of PAGESDIR and PAGES in main.c will allow you to serve up your own content. 

coreshot was created as a skill-development exercise as my first development project using the C language. As such, I do not recommend using any part of it for any reason should anyone ever somehow be tempted. Given the language, the functional role that it is supposed to fill, and my experience-level with C development, it may be the engineering-artifact equivalent of a jet-engine motorcycle made out of paper mache by a kindergardner.

Changes to come
-----------

These are some functionalities, features, or changes that I will make happen soon:

- Switch to poll or epoll

- Create passable logging 

- IPv6 support

Possible future stuff
-----------

- Fork w/ SSL 

- Fork a version that pulls in and utilizes an actual HTTP implementation


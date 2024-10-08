CC=gcc
CFLAGS=-I.
DEPS = main.c http_parser.c http_responder.c logging.c
OBJ = main.o http_parser.o http_responder.o logging.o
OUT = coreshot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ) $(OUT)


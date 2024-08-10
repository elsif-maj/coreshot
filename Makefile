CC=gcc
CFLAGS=-I.
DEPS = main.c http_parser.c
OBJ = main.o http_parser.o
OUT = coreshot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ) $(OUT)


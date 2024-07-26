CC=gcc
CFLAGS=-I.
DEPS = main.c
OBJ = main.o
OUT = coreshot

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ) $(OUT)


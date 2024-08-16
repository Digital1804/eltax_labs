SRC := server.c client.c
OBJ := $(SRC:.c=.o)
BIN := server client
CC := gcc
CFLAGS = -lrt

.PHONY: clean all

all: $(BIN)

$(BIN): %: %.o 
	$(CC) -o $@ $< $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $<
	
clean:
	rm -rf client server *.o
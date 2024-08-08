SRC := server.c client.c
OBJ := $(SRC:.c=.o)
BIN := server client
HED := info.h
CC := gcc
CFLAGS = -lrt -lncurses -lpthread

.PHONY: clean all

all: $(BIN)

$(BIN): %: %.o 
	$(CC) -o $@ $< $(CFLAGS)

%.o: %.c $(HED)
	$(CC) -c $< $(HED)
	
clean:
	rm -rf client server *.o info.h.gch

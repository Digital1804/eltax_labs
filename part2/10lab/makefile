CC = gcc
SRCS = kill_sender.c event_loop.c sigint.c sigusr1.c
TARGETS = $(SRCS:.c=)

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)
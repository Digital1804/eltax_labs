CC = gcc
SRCS_local = tcp_client_local.c tcp_server_local.c udp_server_local.c udp_client_local.c
SRCS_inet = tcp_client_inet.c tcp_server_inet.c udp_server_inet.c udp_client_inet.c

TARGETS = $(SRCS_local:.c=) $(SRCS_inet:.c=)

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)
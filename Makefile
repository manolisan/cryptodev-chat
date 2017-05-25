###################################################
#
# Makefile
# Simple TCP/IP communication using sockets
#
# Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
#
###################################################

CC = gcc

CRYPTODEVDIR=$(HOME)/cryptodev/cryptodev-linux-1.6

CFLAGS = -Wall -I$(CRYPTODEVDIR)
CFLAGS += -g

# CFLAGS += -O2 -fomit-frame-pointer -finline-functions

LIBS = -lreadline

BINS = socket-client socket-server

all: $(BINS)

socket-client: socket-client.o socket-common.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

socket-server: socket-server.o socket-common.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

socket-server.o: socket-server.c
	$(CC) $(CFLAGS) -c $^

socket-client.o: socket-client.c
		$(CC) $(CFLAGS) -c $^

socket-common.o: socket-common.c
	$(CC) $(CFLAGS) -c socket-common.c

clean:
	rm -f *.o *~ $(BINS)

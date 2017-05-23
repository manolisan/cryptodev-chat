###################################################
#
# Makefile
# Simple TCP/IP communication using sockets
#
# Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
#
###################################################

CC = gcc

CFLAGS = -Wall
CFLAGS += -g
# CFLAGS += -O2 -fomit-frame-pointer -finline-functions

LIBS =

BINS = socket-server socket-client

all: $(BINS)

socket-server: socket-server.o socket-common.h socket-common.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

socket-client: socket-client.c socket-common.h socket-common.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

socket-server.o: socket-server.c
	$(CC) $(CFLAGS) -c $^

socket-client.o: socket-client.c
		$(CC) $(CFLAGS) -c $^

socket-common.o: socket-common.c
	$(CC) $(CFLAGS) -c socket-common.c

clean:
	rm -f *.o *~ $(BINS)

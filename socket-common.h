/*
 * socket-common.h
 *
 * Simple TCP/IP communication using sockets
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 */

 #include <ctype.h>
 #include <sys/types.h>

#ifndef _SOCKET_COMMON_H
#define _SOCKET_COMMON_H

/* Compile-time options */
#define TCP_PORT    35001
#define TCP_BACKLOG 5

#define HELLO_THERE "Hello there!"

#endif /* _SOCKET_COMMON_H */

ssize_t insist_write(int fd, const void *buf, size_t cnt);

size_t read_line(char * buf);

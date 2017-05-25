/*
* socket-common.h
*
* Simple TCP/IP communication using sockets
*
* Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
*/
#include <ctype.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <crypto/cryptodev.h>

#ifndef _SOCKET_COMMON_H
#define _SOCKET_COMMON_H

/* Compile-time options */
#define TCP_PORT    35001
#define TCP_BACKLOG 5

#define HELLO_THERE "Hello there!"

#endif /* _SOCKET_COMMON_H */

#define BUFF_SIZE 300
#define DATA_SIZE 256
#define BLOCK_SIZE  16
#define KEY_SIZE	16

extern int newsd;
extern char prompt[100];
extern int encrypted;

ssize_t insist_write(int fd, const void *buf, size_t cnt);

size_t read_line(char * buf);

void read_and_send(char * buf, int sd);

int get_and_print(char * buf, int sd);

void my_rlhandler(char* line);

void encrypt(void *buffer, size_t enc_s);

void decrypt(void *buffer);

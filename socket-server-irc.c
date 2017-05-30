/*
* socket-server.c
* Simple TCP/IP communication using sockets
*
* Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
*/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/select.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <pthread.h>

#include "socket-common.h"
#include <crypto/cryptodev.h>

char * prompt;
int newsd_serve[MAX_CLIENTS];
int newsd;
int encrypted=0;
int no_client;
pthread_t tid[MAX_CLIENTS];

char addrstr[INET_ADDRSTRLEN];
int sd;
socklen_t len;
struct sockaddr_in sa;



int forward(int sd, int me){
	char *buf;
	ssize_t n;
	uint32_t size;

	if(encrypted==1){
		buf = (char *) malloc(sizeof(buf) *BUFF_SIZE);
		if (buf == 0)
		{
			printf("ERROR: Out of memory\n");
			exit(1);
		}
		n = insist_read(sd, buf, DATA_SIZE);
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;

		decrypt(buf);

		memcpy(&size, buf, sizeof(size));
		size = ntohl(size);

		buf += sizeof(size);

	}
	else{
		n  = insist_read(sd, &size, sizeof(size));
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;

		size = ntohl(size);

		buf = (char *) malloc(sizeof(buf)*(size + sizeof(size)));
		if (buf == 0)
		{
			printf("ERROR: Out ofsize memory\n");
			exit(1);
		}
		size = htonl(size);
		memcpy(buf, &size, sizeof(size));
		size = ntohl(size);
		buf += sizeof(size);

		n = insist_read(sd, buf, size);
		if (n < 0) {
			perror("read");
			exit(1);
		}
		if (n <= 0) return 1;
	}

	// Write message to stdout
	if (insist_write(0, buf, size) != size) {
		perror("write");
		exit(1);
	}

	buf -= sizeof(size);

	//distribute!!
	int client;
	for(client = 0; client<no_client; client++){
		if (client == me) continue;
		if(encrypted == 1){
			encrypt(buf, size+sizeof(size));
			if (insist_write(newsd_serve[client], buf, DATA_SIZE) != DATA_SIZE) {
				perror("write");
				exit(1);
			}
		} else {
			if (insist_write(newsd_serve[client], buf, size + sizeof(size)) != size + sizeof(size)) {
				perror("write");
				exit(1);
			}
		}
	}


	free(buf);
	return 0;
}

void *client_service(void* client_id){

	fprintf(stderr, "Incoming connection from %s:%d\n",
	addrstr, ntohs(sa.sin_port));
	int id = *((int *) client_id);
	int client_sd = newsd_serve[id];

	printf("From Client %d!\n", id);

	while (forward(client_sd, id) != 1);

	printf("Client %d went away!\n", id);

	/* Make sure we don't leak open files */
	if (close(newsd_serve[no_client]) < 0)
	perror("close");

	return NULL;
}


int main(int argc, char *argv[])
{

	if (argc != 2 && argc != 1){
		fprintf(stderr, "Usage: %s [--encrypted]\n", argv[0]);
		exit(1);
	}

	//Check for encryption option
	if (argc == 2){
		if (strcmp(argv[1], "--encrypted")==0) encrypted = 1;
	}

	/* Make sure a broken connection doesn't kill us */
	signal(SIGPIPE, SIG_IGN);

	/* Create TCP/IP socket, used as main chat channel */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	fprintf(stderr, "Created TCP socket\n");

	/* Bind to a well-known port */
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(TCP_PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		perror("bind");
		exit(1);
	}
	fprintf(stderr, "Bound TCP socket to port %d\n", TCP_PORT);

	/* Listen for incoming connections */
	if (listen(sd, TCP_BACKLOG) < 0) {
		perror("listen");
		exit(1);
	}

	/* Loop forever, accept()ing connections */
	no_client = 0;
	for (;;) {
		fprintf(stderr, "Waiting for an incoming connection...\n");

		/* Accept an incoming connection */
		len = sizeof(struct sockaddr_in);
		if ((newsd_serve[no_client] = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
			perror("accept");
			exit(1);
		}
		if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
			perror("could not format IP address");
			exit(1);
		}

		int client_id = no_client;
		int err = pthread_create(&(tid[no_client]), NULL, &client_service, &client_id);
	  if (err != 0)
	     printf("\ncan't create thread :[%s]", strerror(err));
	  else
	     printf("\nThread created successfully\n");

		no_client++;

	}

	/* This will never happen */
	return 1;
}

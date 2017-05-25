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

#include "socket-common.h"
#include <crypto/cryptodev.h>

char * prompt;
int  newsd;
int encrypted=0;

int main(int argc, char *argv[])
{
	char buf[BUFF_SIZE];
	char addrstr[INET_ADDRSTRLEN];
	int sd;
	socklen_t len;
	struct sockaddr_in sa;

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

	/* Identity */
	prompt = malloc(100*sizeof(char));
	printf("Please enter your prompt: ");
	scanf("%s", prompt);

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
	for (;;) {
		fprintf(stderr, "Waiting for an incoming connection...\n");

		/* Accept an incoming connection */
		len = sizeof(struct sockaddr_in);
		if ((newsd = accept(sd, (struct sockaddr *)&sa, &len)) < 0) {
			perror("accept");
			exit(1);
		}
		if (!inet_ntop(AF_INET, &sa.sin_addr, addrstr, sizeof(addrstr))) {
			perror("could not format IP address");
			exit(1);
		}
		fprintf(stderr, "Incoming connection from %s:%d\n",
		addrstr, ntohs(sa.sin_port));

		rl_callback_handler_install(prompt, (rl_vcpfunc_t*) &my_rlhandler);

		fd_set fds;

		for (;;) {
			FD_ZERO(&fds);
			FD_SET(newsd, &fds);
			FD_SET(0, &fds);

			while (select (newsd+1, &fds, 0, 0, 0) < 0) {
				if (errno != EINTR && errno != EAGAIN)
				perror("select");
				exit(1);
			}

			if (FD_ISSET(0, &fds))
			{
				rl_callback_read_char();
			}
			if (FD_ISSET(newsd, &fds)){
				if (get_and_print(buf, newsd) == 1) break;
			}
		}

		printf("Client went away!\n");

		/* Make sure we don't leak open files */
		if (close(newsd) < 0)
		perror("close");

	}

	rl_callback_handler_remove();

	/* This will never happen */
	return 1;
}

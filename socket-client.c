/*
* socket-client.c
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

int newsd;
char *prompt;
int encrypted=0;

int main(int argc, char *argv[])
{

	int port;
	char buf[BUFF_SIZE];
	char *hostname;
	struct hostent *hp;
	struct sockaddr_in sa;

	if (argc != 3 && argc != 4) {
		fprintf(stderr, "Usage: %s hostname port [--encrypted]\n", argv[0]);
		exit(1);
	}
	hostname = argv[1];
	port = atoi(argv[2]); /* Needs better error checking */

	//Check for encryption option
	if (argc == 4){
		if (strcmp(argv[3], "--encrypted")==0) encrypted = 1;
	}

	signal(SIGINT, intHandler);

	prompt = malloc(100*sizeof(char));
	printf("Please enter your prompt: ");

	scanf("%s", prompt);

	/* Create TCP/IP socket, used as main chat channel */
	if ((newsd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	fprintf(stderr, "Created TCP socket\n");

	/* Look up remote hostname on DNS */
	if ( !(hp = gethostbyname(hostname))) {
		printf("DNS lookup failed for host %s\n", hostname);
		exit(1);
	}

	/* Connect to remote TCP port */
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr.s_addr, hp->h_addr, sizeof(struct in_addr));
	fprintf(stderr, "Connecting to remote host... "); fflush(stderr);
	if (connect(newsd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("connect");
		exit(1);
	}
	fprintf(stderr, "Connected.\n");

	/* Be careful with buffer overruns, ensure NUL-termination */
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

	/* Make sure we don't leak open files */
	if (close(newsd) < 0)
		perror("close");

	rl_callback_handler_remove();

	fprintf(stderr, "\nServer abonded connection.\n");
	return 0;
}

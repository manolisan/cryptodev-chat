#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "socket-common.h"


/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt)
{
	ssize_t ret;
	size_t orig_cnt = cnt;

	while (cnt > 0) {
	        ret = write(fd, buf, cnt);
	        if (ret < 0)
	                return ret;
	        buf += ret;
	        cnt -= ret;
	}
	return orig_cnt;
}

int get_and_print(char * buf, int sd){
	/// sizeof buf??? when o put the change of line?
  ssize_t n = read(sd, buf, sizeof(buf));

  if (n < 0) {
    perror("read");
    exit(1);
  }

	//printf("Received %d bytes:\n", n);

  //If reached EOF, return 1.
  if (n <= 0)
    return 1;

	int saved_point = rl_point;
	char *saved_line = rl_copy_text(0, rl_end);
	rl_save_prompt();
	rl_replace_line("", 0);
	rl_redisplay();


	/* Write message to stdout */
  if (insist_write(0, buf, n) != n) {
    perror("write");
    exit(1);
  }
	//printf("\n");

	rl_restore_prompt();
	rl_replace_line(saved_line, 0);
	rl_point = saved_point;
	rl_redisplay();
	free(saved_line);

  return 0;
}

void my_rlhandler(char* line)
{
    if (line == NULL)
        return ; //???? to change!!!!!

    size_t len = strlen(line);
		size_t name_len = strlen(prompt);
    if (*line != 0) {
      add_history(line);



			// format messasge
			char * full_message = malloc(len + name_len + 2); // 1 for a space, 1 for line changing and 1 for a \0
 			strcpy(full_message, prompt);
 			full_message[name_len] = ' ';
			strcpy(full_message + name_len + 1, line);
 			full_message[len +name_len + 1] = '\n';
			//full_message[len +name_len + 3] = '\0';


			if(insist_write(newsd, full_message, len + name_len + 2) != len + name_len + 2){
				perror("write");
				exit(1);
			}
			free(full_message);

/*
			// or just send the line!
			if(insist_write(newsd, line, len) != len){
				perror("write");
				exit(1);
			}
*/

			printf("\n");			//print one change line on each message send for readability
			fflush(stdout);

    }

    free (line);
}

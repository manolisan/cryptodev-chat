#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
  ssize_t n = read(sd, buf, sizeof(buf));

  if (n < 0) {
    perror("read");
    exit(1);
  }

  //If reached EOF, return 1.
  if (n <= 0)
    return 1;

	printf("--Interrupted print\n");
	int saved_point = rl_point;
	char *saved_line = rl_copy_text(0, rl_end);
	rl_save_prompt();
	rl_replace_line("", 0);
	rl_redisplay();

	printf("Test saved line:%s", saved_line);
  printf(":END\n");
	/* Write message to stdout */
  if (insist_write(0, buf, n) != n) {
    perror("write");
    exit(1);
  }

	printf("--Back again? :\n");

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
        return ; //to change!!!!!
		printf("We have an Interrupt!!!\n");
    size_t len = strlen(line);
    if (*line != 0)
    {
      add_history(line);

      if(insist_write(newsd, line, len) != len ){
				perror("write");
				exit(1);
			}
    }
		fprintf(stdout, "I said:\n%s\nRemote says:\n", line);
		fflush(stdout);

    free (line);
}

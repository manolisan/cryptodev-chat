#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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

size_t read_line(char * buf){
  size_t cnt = 0;
  char c;
  while(((c=getchar()) != '\n') && cnt < 100){
    buf[cnt]=c;
    cnt++;
  }
  buf[++cnt] = '\n';
  buf[++cnt] = '\0';
  return cnt;
}

void read_and_send(char * buf, int sd){
  size_t cnt = read_line(buf);

	if (insist_write(sd, buf, cnt) != cnt) {
		perror("write");
		exit(1);
	}

	fprintf(stdout, "I said:\n%s\nRemote says:\n", buf);
	fflush(stdout);
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

  if (insist_write(0, buf, n) != n) {
    perror("write");
    exit(1);
  }
  return 0;
}

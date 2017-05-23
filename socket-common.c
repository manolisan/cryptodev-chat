#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

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
  buf[++cnt] = '\0';
  return cnt;
}

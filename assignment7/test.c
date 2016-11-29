#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>

int main() {
  int fd;
  if ((fd = open("storage/test_file", O_CREAT | O_EXCL | O_RDWR, 0600)) == -1) {
    perror("open");
    exit(0);
  }
  return 0;
}

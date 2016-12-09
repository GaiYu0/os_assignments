#include<dirent.h>
#include<errno.h>
#include<fcntl.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<sys/file.h>
#include<sys/resource.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

/*
int main() {
  struct rlimit limit;
  getrlimit(RLIMIT_NOFILE, &limit);
  printf("%ld %ld\n", limit.rlim_cur, limit.rlim_max);
  return 0;
}
*/

char *content = "Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal. \
Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. \
But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, under God, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not perish from the earth.";

void *thread(void *pointer) {
  int fd;
  srand(pthread_self());
  double value = ((double)rand()) / RAND_MAX;
  if (value < 0.5) {
    printf("write\n");
    fd = wopen("__file__", O_WRONLY, S_IRUSR | S_IWUSR);
    int i;
    for (i = 0; i != strlen(content) + 1; i++) {
      write(fd, content + i, sizeof(char));
    }
    wclose("__file__");
  } else {
    printf("read\n");
    char *file;
    asprintf(&file, "gettysburg%d", pthread_self());
    fd = ropen("__file__", O_RDONLY, S_IRUSR);
    int output_fd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    size_t size = (strlen(content) + 1) * sizeof(char);
    size_t length;
    char buffer;
    while (size > 0) {
      length = read(fd, &buffer, sizeof(char));
      write(output_fd, &buffer, length);
      size -= length;
    }
    close(output_fd);
    free(file);
    rclose("__file__");
  }
  close(fd);

  return NULL;
}

int main(int argc, char *argv[]) {
  construct_global_file_lock();
  int N = atoi(argv[1]);
  pthread_t *tids = (pthread_t*)malloc(N * sizeof(pthread_t));
  int i;
  int fd = open("__file__", O_CREAT, S_IRUSR | S_IWUSR);
  for (i = 0; i != N; i++)
    pthread_create(tids + i, NULL, &thread, NULL);
  for (i = 0; i != N; i++) {
    pthread_join(tids[i], NULL);
  }
  close(fd);
  free(tids);
  destroy_global_file_lock();
  return 0;
}

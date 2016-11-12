#include<fcntl.h>
#include<sys/mman.h>
#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

int N;
char **args;

int *fd_table;
int *index_table;
int *length_table;
char **memory;

void cleanup(int);
void *convert(void*);

int main(int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = &cleanup;
  sigaction(SIGINT, &action, NULL);

  pthread_t *tids;
  tids = (pthread_t*)malloc((argc - 1) * sizeof(pthread_t));
  fd_table = (int*)calloc((argc - 1), sizeof(int));
  index_table = (int*)malloc((argc - 1) * sizeof(int));
  length_table = (int*)malloc((argc - 1) * sizeof(int));
  memory = (char**)calloc((argc - 1), sizeof(char*));
  N = argc - 1;
  args = argv;
  int i;
  for (i = 0; i != N; i++) {
    index_table[i] = i;
    if (pthread_create(tids + i, NULL, &convert, (void*)(index_table + i)) != 0) {
      perror("pthread_create");
      kill(getpid(), SIGINT);
    }
  }
  for (i = 0; i != N; i++) {
    if (pthread_join(tids[i], NULL) != 0) {
      perror("pthread_join");
      kill(getpid(), SIGINT);
    }
  }
  printf("end of conversion\n");
  cleanup(SIGINT);
  return 0;
}

void cleanup(int signal) {
  int i;
  for (i = 0; i != N; i++) {
    if (memory[i] != NULL) {
      if (munmap(memory[i], length_table[i]) != 0) {
        perror("munmap");
      }
      if (fd_table[i] > 0) {
        if (close(fd_table[i]) != 0) {
          perror("close");
        }
      }
    }
  }
  exit(0);
}

void *convert(void *pointer) {
  int index;
  index = *((int*)pointer);
  fd_table[index] = open(args[index + 1], O_RDWR);
  if (fd_table[index] < 0) {
    perror("open");
    kill(getpid(), SIGINT);
  }
  length_table[index] = lseek(fd_table[index], 0, SEEK_END);
  memory[index] = (char*)mmap(
    NULL,
    length_table[index],
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    fd_table[index],
    0
  );
  int i;
  for (i = 0; i != length_table[index]; i++) {
    if ((96 < memory[index][i]) && (memory[index][i] < 123)) {
      memory[index][i] -= 32;
    }
  }
  return NULL;
}

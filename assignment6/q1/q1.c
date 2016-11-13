#include<fcntl.h>
#include<sys/mman.h>
#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

int N; // the number of files
char **args; // file names

int *fd_table; // one file descriptor for every file
// index enables every thread to access its data in args, fd_table, length_table and memory
int *index_table;
int *length_table; // file lengths
char **memory; // mmap pointers

void cleanup(int); // SIGINT
void *convert(void*); // threads responsible for conversion

int main(int argc, char *argv[]) {
  // setup cleanup handler
  struct sigaction action;
  action.sa_handler = &cleanup;
  sigaction(SIGINT, &action, NULL);

  // allocate arrays
  pthread_t *tids;
  tids = (pthread_t*)malloc((argc - 1) * sizeof(pthread_t));
  fd_table = (int*)calloc((argc - 1), sizeof(int));
  index_table = (int*)malloc((argc - 1) * sizeof(int));
  length_table = (int*)malloc((argc - 1) * sizeof(int));
  memory = (char**)calloc((argc - 1), sizeof(char*));

  N = argc - 1;
  printf("%d\n", N);
  args = argv;

  int i;
  for (i = 0; i != N; i++) { // create a thread for every file
    index_table[i] = i;
    if (pthread_create(tids + i, NULL, &convert, (void*)(index_table + i)) != 0) {
      perror("pthread_create");
      kill(getpid(), SIGINT);
    }
  }

  for (i = 0; i != N; i++) { // wait for the end of all conversions
    if (pthread_join(tids[i], NULL) != 0) {
      perror("pthread_join");
      kill(getpid(), SIGINT);
    }
  }
  printf("end of conversion\n");

  // release resources
  cleanup(SIGINT);

  return 0;
}

void cleanup(int signal) {
  int i;
  for (i = 0; i != N; i++) {
    if (memory[i] != NULL) { // destroy mmap pointers
      if (munmap(memory[i], length_table[i]) != 0) {
        perror("munmap");
      }
      if (fd_table[i] > 0) { // close files
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
  index = *((int*)pointer); // extract the index
  fd_table[index] = open(args[index + 1], O_RDWR); // open file
  if (fd_table[index] < 0) {
    perror("open");
    kill(getpid(), SIGINT);
  }
  length_table[index] = lseek(fd_table[index], 0, SEEK_END); // find the length of file
  // map file to memory
  memory[index] = (char*)mmap(
    NULL,
    length_table[index],
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    fd_table[index],
    0
  );
  int i;
  for (i = 0; i != length_table[index]; i++) { // convert file
    if ((96 < memory[index][i]) && (memory[index][i] < 123)) {
      memory[index][i] -= 32;
    }
  }
  return NULL;
}

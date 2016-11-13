#include<fcntl.h>
#include<sys/mman.h>
#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

#define N_THREADS 16

int N; // the total number of files
char **args; // file names

int file_index; // the next file to convert
pthread_mutex_t mfiles = PTHREAD_MUTEX_INITIALIZER;

int fd_table[N_THREADS]; // one file descriptor for every thread
// index enables every thread to access its data in args, fd_table, length_table and memory
int index_table[N_THREADS];
int length_table[N_THREADS]; // file length, one per thread
char *memory[N_THREADS]; // mmap pointers

void cleanup(int); // SIGINT
void *convert(void*); // threads responsible for conversion

int main(int argc, char *argv[]) {
  // setup cleanup handler
  struct sigaction action;
  action.sa_handler = &cleanup;
  sigaction(SIGINT, &action, NULL);

  N = argc - 1;
  args = argv + 1;

  // create N_THREADS threads
  int i;
  pthread_t tid_table[N_THREADS];
  for (i = 0; i != N_THREADS; i++) {
    index_table[i] = i;
    if (pthread_create(tid_table + i, NULL, &convert, (void*)(index_table + i)) != 0) {
      perror("pthread_create");
      kill(getpid(), SIGINT);
    }
  }
  // wait for the end of conversion
  for (i = 0; i != N_THREADS; i++) {
    if (pthread_join(tid_table[i], NULL) != 0) {
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
  for (i = 0; i != N_THREADS; i++) {
    if (memory[i] != NULL) { // release mapped memory
      if (munmap(memory[i], length_table[i]) != 0) {
        perror("munmap");
      }
      if (fd_table[i] > 0) { // close file
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
  while (1) { // continuously convert the next file until there is no file to convert
    pthread_mutex_lock(&mfiles);
    if (file_index == N) { // no file to convert
      pthread_mutex_unlock(&mfiles);
      return NULL;
    }
    fd_table[index] = open(args[file_index], O_RDWR); // open file
    file_index += 1; // index points to the next file available
    pthread_mutex_unlock(&mfiles);

    length_table[index] = lseek(fd_table[index], 0, SEEK_END); // file length
    // map file to memory
    memory[index] = (char*)mmap(
      NULL,
      length_table[index],
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      fd_table[index],
      0
    );

    // convert
    int i;
    for (i = 0; i != length_table[index]; i++) {
      if ((96 < memory[index][i]) && (memory[index][i] < 123)) {
        memory[index][i] -= 32;
      }
    }

    // cleanup
    munmap(memory[index], length_table[index]);
    memory[index] = NULL;
    close(fd_table[index]);
    fd_table[index] = 0;
  }
  return NULL;
}

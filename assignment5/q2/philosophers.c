#define _GNU_SOURCE
#include<errno.h>
#include<fcntl.h>
#include<math.h>
#include<semaphore.h>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/stat.h>

#define ARRAY "/array"
#define ARRAY_SEMAPHORE "/array-semaphore"
#define CHOPSTICK_SEMAPHORE "/chopstick-semaphore-%d"
#define PHILOSOPHER_SEMAPHORE "/philosopher-semaphore-%d"

int N;
sem_t **schopsticks;
sem_t **sphilosophers;

int *array;
sem_t *sarray;

void cleanup(int signal);
void print_array(int *, int length);

int main(int argc, char *argv[]) {
  int times;
  N = atoi(argv[1]);
  times = atoi(argv[2]);

  // open memory for visualization
  int fd;
  fd = shm_open(ARRAY, O_CREAT | O_RDWR, 0600);
  if (ftruncate(fd, N * sizeof(int)) == -1) {
    perror("ftruncate");
    exit(0);
  }
  array = (int*)mmap(
    NULL,
    N * sizeof(int),
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    fd,
    0
  );
  sarray = sem_open(ARRAY_SEMAPHORE, O_CREAT | O_RDWR, 0666, 1);

  // open semaphore
  schopsticks = (sem_t**)malloc(N * sizeof(sem_t*));
  sphilosophers = (sem_t**)malloc((int)ceil(N / 2.0) * sizeof(sem_t*));
  int i, index;
  char *path;
  for (i = 0; i != N; i++) {
    asprintf(&path, CHOPSTICK_SEMAPHORE, i);
    schopsticks[i] = sem_open(path, O_CREAT | O_RDWR, 0666, 1);
    if (i % 2 == 0) {
      index = i / 2;
      asprintf(&path, PHILOSOPHER_SEMAPHORE, index);
      sphilosophers[index] = sem_open(path, O_CREAT | O_RDWR, 0666, 1);
    }
  }
    

  // register cleanup function
  struct sigaction action;
  action.sa_handler = &cleanup;
  sigaction(SIGINT, &action, NULL);

  // fork
  pid_t pids[N];
  for (i = 0; i != N; i++) {
    if ((pids[i] = fork()) == 0) {
      break;
    }
  }
  
  // start simulation
  if (i == N) { // parent
    int status;
    for (i = 0; i != N; i++) {
      wait(&status);
    }
    printf("finished\n");
  } else { // child, i.e. philosopher
    int left, right;
    left = i;
    right = (i + 1) % N;
    index = (int)ceil(i / 2.0);
    int counter;
    for (counter = 0; counter != times; counter++) {
      // acquire chopsticks
      sem_wait(sphilosophers[index]);
      sem_wait(schopsticks[left]);
      sem_wait(schopsticks[right]);
      sem_post(sphilosophers[index]);

      // logging
#if DETAILS
      printf("philosopher %d %d start\n", i, counter);
#endif
      sem_wait(sarray);
      array[i] = 1;
      print_array(array, N);
      sem_post(sarray);

      // dine
      sleep(INTERVAL); // TODO random dinning time

      // logging
      sem_wait(sarray);
      array[i] = 0;
      print_array(array, N);
      sem_post(sarray);
#if DETAILS
      printf("philosopher %d %d end\n", i, counter);
#endif

      // release chopsticks
      sem_post(schopsticks[right]);
      sem_post(schopsticks[left]);
    }
  }

  // cleanup
  cleanup(SIGINT);

  return 0;
}

void cleanup(int signal) {
  // semaphore
  int i, index;
  char *path;
  for (i = 0; i != N; i++) {
    sem_close(schopsticks[i]);
    asprintf(&path, CHOPSTICK_SEMAPHORE, i);
    sem_unlink(path);
    if (i % 2 == 0) {
      index = i / 2;
      sem_close(sphilosophers[index]);
      asprintf(&path, PHILOSOPHER_SEMAPHORE, index);
      sem_unlink(path);
    }
  }
  free(sphilosophers);
  free(schopsticks);

  sem_close(sarray);
  sem_unlink(ARRAY_SEMAPHORE);

  // array
  munmap(array, N * sizeof(int));
  shm_unlink(ARRAY);
}

void print_array(int *array, int length) {
  char *buffer;
  buffer = (char*)malloc(length * sizeof(char));
  int i;
  for (i = 0; i != length - 1; i++) {
    buffer[i * 2] = '0' + array[i];
    buffer[i * 2 + 1] = ' ';
  }
  buffer[i * 2] = '0' + array[i];
  printf("%s\n", buffer);
  free(buffer);
}

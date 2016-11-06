#include<header.h>

char *server_memory, *client_memory;
char *squeue_path, *srequest_path, *saccept_path, *sresult_path;
sem_t *squeue, *srequest, *saccept, *sresult;
request_queue_t *queue;
float *result;

int initialize_shared_memory(int server_id, int client_id);
int initialize_semaphore(int server_id, int client_id);
void cleanup(int signal);

int main(int argc, char *argv[]) {
  // read inputs
  int server_id, client_id, from, to;
  float value;
  server_id = atoi(argv[1]);
  client_id = atoi(argv[2]);
  from = currency_index(argv[3]);
  to = currency_index(argv[4]);
  value = atof(argv[5]);
#if DEBUG
  printf("input parsed\n");
#endif

  // initialize
  initialize_shared_memory(server_id, client_id);
  initialize_semaphore(server_id, client_id);
#if DEBUG
  printf("client initialized\n");
#endif

  // register cleanup function
  struct sigaction action;
  action.sa_handler = &cleanup;
  sigaction(SIGINT, &action, NULL);

  // request to initiate conversion
  sem_wait(srequest); 
  request_t request;
  request.client_id = client_id;
  request.from = from;
  request.to = to;
  request.value = value;
  sem_wait(squeue); // TODO squeue
  enqueue(queue, &request);
  sem_post(squeue);
  sem_post(saccept);
#if DEBUG
  printf("request sent\n");
#endif

#if DEBUG
  printf("waiting for response from server\n");
#endif
  sem_wait(sresult);
  printf(
    "%f %s = %f %s\n",
    (float)value,
    argv[3],
    *result,
    argv[4]
  );

  cleanup(SIGINT);

  return 0;
}

int initialize_shared_memory(int server_id, int client_id) {
  int server_fd, client_fd;
  asprintf(&server_memory, "/server_%d_shm:0", server_id);
  server_fd = shm_open(server_memory, O_RDWR, 0600);
  if (server_fd == -1) {
    perror("shm_open server");
    exit(1);
  }
  queue = (request_queue_t*)mmap(
    NULL,
    sizeof(request_queue_t),
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    server_fd,
    0
  );
  asprintf(&client_memory, "client_%d_shm:%d", server_id, client_id);
  client_fd = shm_open(client_memory, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (client_fd == -1) {
    if (errno != EEXIST) {
      perror("shm_open client");
      exit(1);
    } else {
      perror("duplicated client");
      exit(1);
    }
  }
  if (ftruncate(client_fd, sizeof(float)) == -1) {
    perror("client truncate");
    exit(1);
  }
  result = (float*)mmap(
    NULL,
    sizeof(float),
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    client_fd,
    0
  );
  return 0;
}

int initialize_semaphore(int server_id, int client_id) {
  asprintf(&squeue_path, "/queue_%d_semaphore", server_id);
  squeue = sem_open(squeue_path, O_RDWR);
  if (squeue == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }
  asprintf(&srequest_path, "/request_%d_semaphore", server_id);
  srequest = sem_open(srequest_path, O_RDWR);
  if (srequest == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }
  asprintf(&saccept_path, "/accept_%d_semaphore", server_id);
  saccept = sem_open(saccept_path, O_RDWR);
  if (saccept == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  asprintf(&sresult_path, "/client_%d_semaphore:%d", server_id, client_id);
  sresult = sem_open(sresult_path, O_CREAT | O_EXCL | O_RDWR, 0666, 0);
  if (sresult == SEM_FAILED) {
    if (errno != EEXIST) {
        perror("sem_open");
        exit(1);
    } else {
      perror("duplicated client");
      exit(1);
    }
  }
  return 0;
}

void cleanup(int signal) {
  sem_close(squeue);
  sem_close(srequest);
  sem_close(saccept);
  sem_close(sresult);

  sem_unlink(sresult_path);

  free(squeue_path);
  free(srequest_path);
  free(saccept_path);
  free(sresult_path);

  munmap(queue, sizeof(request_queue_t));
  munmap(result, sizeof(float));

  shm_unlink(client_memory);

  exit(0);
}

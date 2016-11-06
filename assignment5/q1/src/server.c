#include<header.h>

// master memory
char *server_memory;
char *squeue_path, *srequest_path, *saccept_path;
sem_t *squeue, *srequest, *saccept;
request_queue_t *queue;

// worker memory
char *worker_memory[N_CURRENCY];
char *swqueue_path[N_CURRENCY], *swrequest_path[N_CURRENCY], *swaccept_path[N_CURRENCY];
sem_t *swqueue[N_CURRENCY], *swrequest[N_CURRENCY], *swaccept[N_CURRENCY];
request_queue_t *wqueue[N_CURRENCY];

// client memory
char *client_memory;
char *sresult_path;
sem_t *sresult;
float *result;

int server_initialize_shared_memory(int server_id);
int server_initialize_semaphore(int server_id);
int setup_workers(int server_id);
int return_result(int server_id, int client_id, float value);
void cleanup(int signal);
void cleanup_client_memory();
void cleanup_client_semaphore();
void cleanup_workers();

int main(int argc, char *argv[]) {
  int server_id;
  server_id = atoi(argv[1]);

  server_initialize_shared_memory(server_id);
  server_initialize_semaphore(server_id);
  setup_workers(server_id);
  printf("server initialized\n");

  // register cleanup function
  struct sigaction action;
  action.sa_handler = &cleanup;
  int signal;
  for (signal = 0; signal != NSIG; signal++) {
    sigaction(signal, &action, NULL);
  }

  // fork
  pid_t pids[N_CURRENCY];
  int i;
  for (i = 0; i != N_CURRENCY; i++) {
    pids[i] = fork();
    if (pids[i] == 0) {
      break;
    }
  }

  request_t request;
  if (i == N_CURRENCY) { // master
    while (1) {
      sem_wait(saccept);
      sem_wait(squeue);
      dequeue(queue, &request);
      sem_post(squeue);
      sem_post(srequest);
      
      // logging
      printf(
        "request accepted(client %d from %s to %s value %f)\n",
        request.client_id,
        currency_table[request.from],
        currency_table[request.to],
        request.value
      );

      // distribute request to workers
#if DEBUG
      printf("distributing request\n");
#endif
      sem_wait(swrequest[request.to]);
      sem_wait(swqueue[request.to]);
      enqueue(wqueue[request.to], &request);
      sem_post(swqueue[request.to]);
      sem_post(swaccept[request.to]);
    } // while
  } else { // worker
#if DEBUG
    printf("worker %d started\n", i);
#endif
    while (1) {
      // extract request from queue
      sem_wait(swaccept[i]);
      sem_wait(swqueue[i]);
      dequeue(wqueue[i], &request);
      sem_post(swqueue[i]);
      sem_post(swrequest[i]);
#if DEBUG
      printf("worker %d request received\n", i);
#endif

      // convert
      float value;
      value = convert_to(request.value, request.from, request.to);
#if DEBUG
      printf("value converted\n");
#endif

      // return result to client
#if DEBUG
      printf("returning converted value to client %d\n", request.client_id);
#endif
      return_result(server_id, request.client_id, value);
#if DEBUG
      printf("converted value returned to client %d\n", request.client_id);
#endif
    }
  }
    
  cleanup(SIGINT);

  return 0;
}

int server_initialize_shared_memory(int server_id) {
  int server_fd, client_fd;
  asprintf(&server_memory, "/server_%d_shm:0", server_id);
  server_fd = shm_open(server_memory, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (server_fd == -1) {
    perror("shm_open server");
    exit(1);
  }
  if (ftruncate(server_fd, sizeof(request_queue_t)) == -1) {
    perror("ftruncate");
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
  return 0;
}

int server_initialize_semaphore(int server_id) {
  asprintf(&squeue_path, "/queue_%d_semaphore", server_id);
  squeue = sem_open(squeue_path, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
  if (squeue == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }
  asprintf(&srequest_path, "/request_%d_semaphore", server_id);
  srequest = sem_open(srequest_path, O_CREAT | O_EXCL | O_RDWR, 0666, QUEUE_LENGTH);
  if (srequest == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }
  asprintf(&saccept_path, "/accept_%d_semaphore", server_id);
  saccept = sem_open(saccept_path,  O_CREAT | O_EXCL | O_RDWR, 0666, 0);
  if (saccept == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }

  return 0;
}

int setup_workers(int server_id) {
  int i;
  for (i = 0; i != N_CURRENCY; i++) {
    // queue
    asprintf(&worker_memory[i], "/worker_%d_shm:%d", server_id, i);
    int fd;
    fd = shm_open(worker_memory[i], O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
      if (errno == EEXIST) {
        printf("duplicated server\n");
      } else {
        perror("client shm_open");
      }
      exit(1);
    }
    ftruncate(fd, sizeof(request_queue_t));
    wqueue[i] = (request_queue_t*)mmap(
      NULL,
      sizeof(request_queue_t),
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      fd,
      0
    );

    // swaccept
    asprintf(&swaccept_path[i], "/waccept_%d_semaphore:%d", server_id, i);
    swaccept[i] = sem_open(swaccept_path[i],  O_CREAT | O_EXCL | O_RDWR, 0666, 0);
    if (swaccept[i] == SEM_FAILED) {
      perror("sem_open");
      exit(1);
    }

    // swrequest
    asprintf(&swrequest_path[i], "/wrequest_%d_semaphore:%d", server_id, i);
    swrequest[i] = sem_open(swrequest_path[i],  O_CREAT | O_EXCL | O_RDWR, 0666, N_CURRENCY);
    if (swrequest[i] == SEM_FAILED) {
      perror("sem_open");
      exit(1);
    }

    // swqueue
    asprintf(&swqueue_path[i], "/wqueue_%d_semaphore:%d", server_id, i);
    swqueue[i] = sem_open(swqueue_path[i],  O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (swqueue[i] == SEM_FAILED) {
      perror("sem_open");
      exit(1);
    }
  } // for

  return 0;
}

int return_result(int server_id, int client_id, float value) {
  // client semaphore
  asprintf(&sresult_path, "/client_%d_semaphore:%d", server_id, client_id);
  sresult = sem_open(sresult_path, O_RDWR);
  if (sresult == SEM_FAILED) {
    perror("terminated client");
    exit(1);
  }
#if DEBUG
  printf("client semaphore opened\n");
#endif
  // client memory
  asprintf(&client_memory, "client_%d_shm:%d", server_id, client_id);
  int client_fd;
  client_fd = shm_open(client_memory, O_RDWR, 0600);
  if (client_fd == -1) {
    perror("shm_open client");
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
#if DEBUG
  printf("client memory opened\n");
#endif
  *result = value;
#if DEBUG
  printf("result written\n");
#endif
  sem_post(sresult);
  cleanup_client_memory();
  cleanup_client_semaphore();
  return 0;
}

void cleanup_client_semaphore() {
  sem_close(sresult);
  sem_unlink(sresult_path);
  free(sresult_path);
  sresult_path = NULL; // indicate the end of sharing
}

void cleanup_client_memory() {
  munmap(result, sizeof(float));
  shm_unlink(client_memory);
  free(client_memory);
  client_memory = NULL;
}

void cleanup_workers() {
  int i;
  for (i = 0; i != N_CURRENCY; i++) {
    // memory
    if (wqueue[i]) { munmap(wqueue[i], sizeof(request_queue_t)); }
    if (worker_memory[i]) { shm_unlink(worker_memory[i]); }

    // swrequest
    if (swrequest[i]) { sem_close(swrequest[i]); }
    if (swrequest_path[i]) {
      sem_unlink(swrequest_path[i]);
      free(swrequest_path[i]);
    }

    // swaccept
    if (swaccept[i]) { sem_close(swaccept[i]); }
    if (swaccept_path[i]) {
      sem_unlink(swaccept_path[i]);
      free(swaccept_path[i]);
    }

    // swqueue
    if (swqueue[i]) { sem_close(swqueue[i]); }
    if (swqueue_path[i]) {
      sem_unlink(swqueue_path[i]);
      free(swqueue_path[i]);
    }
  } // for
}
void cleanup(int signal) {
  // semaphore queue
  sem_close(squeue);
  sem_unlink(squeue_path);
  free(squeue_path);

  // semaphore request
  sem_close(srequest);
  sem_unlink(srequest_path);
  free(srequest_path);

  // semaphore accept
  sem_close(saccept);
  sem_unlink(saccept_path);
  free(saccept_path);

  // shared memory queue
  munmap(queue, sizeof(request_queue_t));
  shm_unlink(server_memory);

  if (client_memory) { // shared memory client
    cleanup_client_memory();
  }
  if (sresult_path) { // semaphore sresult
    cleanup_client_semaphore();
  }

  cleanup_workers();

  exit(0);
}

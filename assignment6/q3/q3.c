#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

// LIFO queue
#define CAPACITY 16
float queue[CAPACITY];
int length; // the length of queue
pthread_mutex_t mqueue = PTHREAD_MUTEX_INITIALIZER; // lock queue
pthread_cond_t cproducer = PTHREAD_COND_INITIALIZER; // could produce
pthread_cond_t cconsumer = PTHREAD_COND_INITIALIZER; // could consume
// asynchronous queue primitive
int enqueue(float*, float, pthread_mutex_t*);
float dequeue(float*, pthread_mutex_t*);

void *producer(void*); // producer thread
void *consumer(void*); // consumer thread

int main(int argc, char *argv[]){
  // create N_PRODUCERS producers and N_CONSUMERS consumers
  srand(getpid());
  int N_PRODUCERS, N_CONSUMERS;
  N_PRODUCERS = atoi(argv[1]);
  N_CONSUMERS = atoi(argv[1]);
  pthread_t *ptids, *ctids;
  ptids = (pthread_t*)calloc(N_PRODUCERS, sizeof(pthread_t));
  ctids = (pthread_t*)calloc(N_CONSUMERS, sizeof(pthread_t));
  int i;
  for (i = 0; i != N_PRODUCERS; i++) {
    pthread_create(ptids + i, NULL, &producer, NULL);
  }
  for (i = 0; i != N_CONSUMERS; i++) {
    pthread_create(ctids + i, NULL, &consumer, NULL);
  }

  // wait
  for (i = 0; i != N_PRODUCERS; i++) {
    pthread_join(ptids[i], NULL);
  }
  for (i = 0; i != N_CONSUMERS; i++) {
    pthread_join(ctids[i], NULL);
  }
  return 0;
}

int enqueue(float *queue, float value, pthread_mutex_t *m) {
  // enqueue blocks if the length of queue reaches its maximum capacity
  if (length == CAPACITY) {
    pthread_cond_wait(&cproducer, m);
  }
  int index;
  queue[length] = value;
  length++;
  pthread_cond_signal(&cconsumer); // unblock a blocked consumer
  return 0;
}

float dequeue(float *queue, pthread_mutex_t *m) {
  // dequeue blocks if there is no element to extract from queue
  if (length == 0) {
    pthread_cond_wait(&cconsumer, m);
  }
  length--;
  float value;
  value = queue[length];
  pthread_cond_signal(&cproducer); // unblock a blocked producer
  return value;
}

void *producer(void *pointer) { // produce a random value
  // lock queue to ensure that the order of printing is identical to the order of queue operation
  pthread_mutex_lock(&mqueue);
  float value;
  value = 100.0 * rand() / RAND_MAX;
  printf("produce %f\n", value);
  enqueue(queue, value, &mqueue);
  pthread_mutex_unlock(&mqueue);
  return NULL;
}

void *consumer(void *pointer) { // extract a value from queue and display the value
  // lock queue to ensure that the order of printing is identical to the order of queue operation
  pthread_mutex_lock(&mqueue);
  float value;
  value = dequeue(queue, &mqueue);
  printf("consume %f\n", value);
  pthread_mutex_unlock(&mqueue);
  return NULL;
}

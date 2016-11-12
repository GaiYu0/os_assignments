#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

#define CAPACITY 16
float queue[CAPACITY];
int front;
int length;
pthread_mutex_t mqueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cproducer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cconsumer = PTHREAD_COND_INITIALIZER;
int enqueue(float*, float);
float dequeue(float*);

void *producer(void*);
void *consumer(void*);

int main(int argc, char *argv[]){
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
  for (i = 0; i != N_PRODUCERS; i++) {
    pthread_join(ptids[i], NULL);
  }
  for (i = 0; i != N_CONSUMERS; i++) {
    pthread_join(ctids[i], NULL);
  }
  return 0;
}

int enqueue(float *queue, float value) {
  pthread_mutex_lock(&mqueue);
  if (length == CAPACITY) {
    pthread_cond_wait(&cproducer, &mqueue);
  }
  int index;
  index = (front + length) % CAPACITY;
  queue[index] = value;
  length++;
  pthread_cond_signal(&cconsumer);
  pthread_mutex_unlock(&mqueue);
  return 0;
}

float dequeue(float *queue) {
  pthread_mutex_lock(&mqueue);
  if (length == 0) {
    pthread_cond_wait(&cconsumer, &mqueue);
  }
  float value;
  value = queue[front];
  front = (front + 1) % CAPACITY;
  length--;
  pthread_cond_signal(&cproducer);
  pthread_mutex_unlock(&mqueue);
  return value;
}

void *producer(void *pointer) {
  float value;
  value = 100.0 * rand() / RAND_MAX;
  enqueue(queue, value);
  return NULL;
}

void *consumer(void *pointer) {
  float value;
  value = dequeue(queue);
  printf("%f\n", value);
  return NULL;
}

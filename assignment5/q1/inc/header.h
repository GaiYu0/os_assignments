#define _GNU_SOURCE
#include<errno.h>
#include<fcntl.h>
#include<semaphore.h>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/stat.h>

#define N_CURRENCY 3

extern char *currency_table[N_CURRENCY];
extern float ratio[N_CURRENCY];

int currency_index(char *c);
float convert_to(float value, int from, int to);

typedef struct request_t {
  int client_id;
  int from;
  int to;
  float value;
} request_t;

#define QUEUE_LENGTH 8
typedef struct request_queue_t {
  request_t array[QUEUE_LENGTH];
  int head;
  int length;
} request_queue_t;

int enqueue(request_queue_t*, request_t*);
int dequeue(request_queue_t*, request_t*);

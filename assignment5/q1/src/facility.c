#include<header.h>

char *currency_table[N_CURRENCY] = {
  "EUR",
  "RMB",
  "USD",
};

float ratio[N_CURRENCY] = {0.13, 1.0, 0.15};

int currency_index(char *c) {
  int i;
  for (i = 0; i != N_CURRENCY; i++) {
    if (strcmp(currency_table[i], c) == 0) {
      return i;
    }
  }
  return N_CURRENCY;
}

float convert_to(float value, int from, int to) {
  return value * ratio[to] / ratio[from];
}

int enqueue(request_queue_t *queue, request_t *value) {
  int index;
  index = (queue->head + queue->length) % QUEUE_LENGTH;
  queue->array[index] = *value;
  queue->length++;
  return 0;
}

int dequeue(request_queue_t *queue, request_t *value) {
  *value = queue->array[queue->head];
  queue->head = (queue->head + 1) % QUEUE_LENGTH;
  queue->length--;
  return 0;
}

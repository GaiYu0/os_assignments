#include<stdlib.h>
#include<header.h>

#define INITIAL_CAPACITY 8
#define UNIT sizeof(void*)

int _grow(array_t* array);

int construct_array(array_t* array) {
  array->front = (void**)malloc(INITIAL_CAPACITY * UNIT);
  array->maximum_length = INITIAL_CAPACITY;
  array->length = 0;
  return 0;
}

int destroy_array(array_t* array) {
  int i;
  for (i = 0; i != array->length; i++) {
    free(array->front[i]);
  }
  free(array->front);
  return 0;
}

int array_append(array_t* array, void* element) {
  if (array->length == array->maximum_length) {
    _grow(array);
  }
  array->front[array->length] = element;
  array->length++;
  return 0;
}

void* array_index(array_t* array, int index) {
  if (index < array->length) {
    return array->front[index];
  } else {
    return NULL;
  }
}

int array_delete(array_t* array, int index) {
  if (index < array->length) {
    free(array->front[index]);
    int i;
    for (i = index + 1; i != array->length; i++) {
      array->front[i - 1] = array->front[i];
    }
    array->length--;
    return 0;
  } else {
    return -1;
  } 
}

int _grow(array_t* array) {
  array->maximum_length *= 2;
  array->front = realloc((void*)(array->front), array->maximum_length * UNIT);
  return 0;
}

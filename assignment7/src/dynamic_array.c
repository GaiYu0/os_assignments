#include<stdlib.h>

#include<header.h>

#define INITIAL_CAPACITY 8
#define UNIT sizeof(void*)

int _grow(array_t* array);

int construct_array(array_t* array) {
  array->array = (void**)malloc(INITIAL_CAPACITY * UNIT);
  if (array->array == NULL) { return -1; }
  array->maximum_length = INITIAL_CAPACITY;
  array->length = 0;
  return 0;
}

int destroy_array(array_t* array) {
  int i;
  if (array->array != NULL) {
    free(array->array);
    array->array = NULL;
  }
  return 0;
}

int array_append(array_t* array, void* element) {
  if (array->length == array->maximum_length) {
    if (_grow(array) == -1) { return -1; }
  }
  array->array[array->length] = element;
  array->length++;
  return 0;
}

void* array_index(array_t* array, int index) {
  if (index < array->length) {
    return array->array[index];
  } else {
    return NULL;
  }
}

int array_delete(array_t* array, int index) {
  if (index < array->length) {
    int i;
    for (i = index + 1; i != array->length; i++) {
      array->array[i - 1] = array->array[i];
    }
    array->length--;
    return 0;
  } else {
    return -1;
  } 
}

int array_find_reference(array_t* array, void* reference) {
  int i;
  for (i = 0; i != array->length; i++) {
    if (array->array[i] == reference) {
      return i;
    }
  }
  return -1;
}

int _grow(array_t* array) {
  void *pointer;
  pointer = realloc((void*)(array->array), array->maximum_length * UNIT * 2);
  if (pointer == NULL) { return -1; }
  array->array = pointer;
  array->maximum_length *= 2;
  return 0;
}

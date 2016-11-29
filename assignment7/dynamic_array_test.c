#include<stdio.h>
#include<stdlib.h>
#include<header.h>

#define N 1024

int main() {
  array_t array;
  construct_array(&array);
  int i, *pointer;
  for (i = 0; i != N; i++) {
    APPEND(int, array, i);
  }
  for (i = 0; i != N; i++) {
    printf("%d\n", GET(int, array, i));
  }
  for (i = 0; i != N; i++) {
    array_delete(&array, array.length - 1);
  }
  printf("%d\n", array.length);
  destroy_array(&array);
  return 0;
}

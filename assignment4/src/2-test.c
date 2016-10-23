#include<stdio.h>
#include<time.h>

#include<header.h>

int main() {
  time_t start, end;
  start = time(NULL);
  mysleep(10);
  end = time(NULL);
  printf("sleeping time: %d seconds\n", (int)(end - start));
  return 0;
}

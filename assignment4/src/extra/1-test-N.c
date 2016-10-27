#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include<header.h>

#define N 9

int main() {
  pid_t pid;
  int i;
  for (i = 0; i != N; i++) {
    pid = fork();
    if (pid == 0) {
      // sleep(i + 1);
      exit(i);
    } else {
      printf("%d\n", pid);
    }
  }
  int status;
  for (i = 0; i != N; i++) {
    printf("waiting\n");
    pid = n_wait();
    printf("%d\n", pid);
  }
  return 0;
}

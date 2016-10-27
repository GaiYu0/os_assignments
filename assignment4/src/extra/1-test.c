#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include<header.h>

int main() {
  int pid, terminated_pid;
  pid = fork();
  int status = 0;
  switch (pid) {
  case -1:
    perror("forking error\n");
    exit(-1);
  case 0:
    printf("child terminating\n");
    exit(9);
  default:
    terminated_pid = status_retrieving_wait(&status);
    printf("child termination code %d\n", status);
    printf("pid of the forked child %d\n", pid);
    printf("pid of the terminated child %d\n", terminated_pid);
    break;
  }
  return 0;
}

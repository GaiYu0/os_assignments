#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
  int i, j, pid, exit_code;
  exit_code = 0;
  for (i = 0; i != 3; i++) {
    if ((pid = fork()) == 0) {
      j = 0;
      while((j < i) && ((pid = fork()) == 0))
        j++;
      if (pid != 0) {
        wait(&exit_code);
      }
      printf("exit\n");
      _exit(j);
    }
  } // for
  for (i = 0; i != 3; i++) {
    wait(&exit_code);
  }
  return 0;
}

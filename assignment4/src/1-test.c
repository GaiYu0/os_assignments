#include<signal.h>
#include<stdio.h>
#include<sys/types.h>
#include<time.h>
#include<unistd.h>

#include<header.h>

void handler(int signal) {
  if (signal == SIGUSR1)
    printf("handler called\n");
}

int main() {
  struct sigaction action;
  action.sa_handler = &handler;
  sigaction(SIGUSR1, &action, NULL);

  pid_t pid;
  pid = fork();
  time_t start, end;
  start = time(NULL);
  if (pid > 0) {
    mysleep(1);
  } else {
    mysleep(1024);
  }
  end = time(NULL);
  printf("sleeping time: %d seconds\n", (int)(end - start));
  if (pid > 0)
    kill(pid, SIGUSR1);

  return 0;
}

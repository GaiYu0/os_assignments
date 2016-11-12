#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

void handler(int signal);

int N;

int main(int argc, char *argv[]) {
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  struct sigaction action;
  action.sa_handler = &handler;
  sigaction(SIGINT, &action, NULL);

  N = atoi(argv[1]);

  printf("all descendants created\n");
  sigemptyset(&mask);
  sigsuspend(&mask);

  printf("all descendants terminated\n");
  return 0;
}

void handler(int signal) {
}

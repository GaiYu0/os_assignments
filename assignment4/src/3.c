#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

pid_t parent_pid, self_pid, child_pid;

void handler(int signal) {
  switch (signal) {
  case SIGCHLD:
    // handling the errors of forking children
    exit(-1);
  case SIGUSR1:
    // handling the 1st wave of signals
    if (parent_pid) {
      kill(parent_pid, SIGUSR1);
    } else {
      // start the 2nd wave
      printf("2nd wave started\n");
      kill(child_pid, SIGUSR2);
    }
    pause();
    break;
  case SIGUSR2:
    // handling the 2nd wave of signals
    if (child_pid) {
      kill(child_pid, SIGUSR2);
    } else {
      // start the 3rd wave
      printf("3rd wave started\n");
      kill(parent_pid, SIGINT);
    }
    pause();
    break;
  case SIGINT:
    // handling the 3rd wave of signals
    if (parent_pid) {
      kill(parent_pid, SIGINT);
    } else {
      printf("End of program.\n");
    }
    exit(0);
  }
}


int main(int argc, char *argv[]) {
  int N = 3;
  if (argc > 1) { 
    N = atoi(argv[1]);
  }

  // register the handler for forking error
  sigset_t signal_set;
  sigemptyset(&signal_set);
  struct sigaction action;
  action.sa_flags = 0;
  action.sa_mask = signal_set;
  action.sa_handler = &handler;
  sigaction(SIGCHLD, &action, NULL);
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  int child_index;
  parent_pid = 0;
  self_pid = getpid();
  for (child_index = 0; child_index != N; child_index++) {
    self_pid = getpid();
    child_pid = fork();
    if (child_pid == -1) {
      // forking failed
      printf("forking failed\n");
      exit(-1); // terminate
    } else if (child_pid == 0) {
      // child
      parent_pid = self_pid;
    } else {
      // parent
      break;
    }
  }
  // start the 1st wave
  if (child_index == N) {
    printf("1st wave started\n");
    kill(parent_pid, SIGUSR1);
  }
  pause();
  return 0;
}

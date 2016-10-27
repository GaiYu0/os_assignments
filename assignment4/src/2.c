#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

pid_t parent_pid, self_pid, child_pid;

void handler(int signal) {
  switch (signal) {
  case SIGUSR1: // handling the 1st wave of signals
    if (parent_pid) { // inform the parent
      kill(parent_pid, SIGUSR1);
    } else { // start the 2nd wave
      printf("2nd wave started\n");
      kill(child_pid, SIGUSR2);
    }
    break;
  case SIGUSR2: // handling the 2nd wave of signals
    if (child_pid) { // inform the child
      kill(child_pid, SIGUSR2);
    } else { // start the 3rd wave
      printf("3rd wave started\n");
      kill(parent_pid, SIGINT);
    }
    break;
  case SIGINT: // handling the 3rd wave of signals
    if (parent_pid) { // inform the parent
      kill(parent_pid, SIGINT);
    } else { // the root process terminates
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

  // register handlers
  struct sigaction action;
  action.sa_handler = &handler;
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  int child_index;
  parent_pid = 0;
  self_pid = getpid();
  for (child_index = 0; child_index != N; child_index++) {
    self_pid = getpid();
    child_pid = fork();
    if (child_pid == 0) { // child
      parent_pid = self_pid;
    } else { // parent
      break;
    }
  }

  sigset_t full_mask, null_mask;
  sigfillset(&full_mask);
  sigemptyset(&null_mask);
  sigprocmask(SIG_SETMASK, &full_mask, NULL); // mask all signals

  if (child_index == N) { // start the 1st wave
    printf("1st wave started\n");
    kill(parent_pid, SIGUSR1);
  } else { // wait for the arrival of the 1st wave
    sigsuspend(&null_mask);
  }

  if (parent_pid != 0) { // wait for the arrival of the 2nd wave
    sigsuspend(&null_mask);
  }

  if (child_index != N) { // wait for the arrival of the 3rd wave
    sigsuspend(&null_mask);
  }

  return 0;
}

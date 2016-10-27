#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

void calculate1();
void calculate2();
void handler(int signal) { }

int main(int argc, char* argv[]) {
  // N children
  int N;
  N = atoi(argv[1]);

  // register handler
  struct sigaction action;
  action.sa_handler = &handler;
  sigaction(SIGUSR1, &action, NULL);

  // mask SIGUSR1
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  // fork N children
  int i;
  pid_t ppid, pids[N];
  ppid = getpid();
  for (i = 0; i != N; i++)
    if ((pids[i] = fork()) == 0)
      break;

  calculate1();

  // synchronize
  // nth child -> (n-1)th child -> ... -> 1st child -> parent
  // parent -> 1st child -> ... -> (n-1)th child -> nth child
  sigemptyset(&mask);
  if (i == N) {
    sigsuspend(&mask); // unmask all signals and wait for the 1st child
    kill(pids[0], SIGUSR1);
  } else {
    if (i == N - 1) { // the nth child initiates signal
      kill(pids[N - 2], SIGUSR1);
    } else { // other children wait for a signal from the next child
      sigsuspend(&mask);
      if (i == 0) { // the 1st child signals the parent
        kill(ppid, SIGUSR1);
      } else { // other children signal the previous child
        kill(pids[i - 1], SIGUSR1);
      }
    }
    sigsuspend(&mask); // all children wait for a signal from the previous child
    if (i != N - 1) { // all children signal the next one except for the last child
      kill(pids[i + 1], SIGUSR1); // signal the next child
    }
  }

  calculate2();

  return 0;
}

void calculate1() {
  printf("calculate1\n");
  int i;
  for (i = 0; i != 1E8; i++);
}

void calculate2() {
  printf("calculate2\n");
  int i;
  for (i = 0; i != 1E8; i++);
}

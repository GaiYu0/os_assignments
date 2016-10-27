#include<signal.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

void handler(int);
void calculate1();
void calculate2();

int main() {
  pid_t ppid, pid1, pid2;
  ppid = getpid(); // parent pid

  sigset_t mask;
  sigfillset(&mask);

  struct sigaction action;
  action.sa_handler = &handler;
  action.sa_mask = mask; // mask all signals
  sigaction(SIGUSR1, &action, NULL); // for child 1
  sigaction(SIGUSR2, &action, NULL); // for child 2

  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigprocmask(SIG_SETMASK, &mask, NULL); // mask SIGUSR1, SIGUSR2

  sigset_t empty_set;
  sigemptyset(&empty_set);

  pid1 = fork(); // fork child 1
  if (pid1 != 0) {
    pid2 = fork(); // fork child 2
  }

  calculate1();

  // synchronize
  if (pid1 && pid2) { // parent
    sigsuspend(&empty_set); // unmask all signals and wait
    sigsuspend(&empty_set); // unmask all signals and wait
    kill(pid1, SIGUSR1); // inform child1
    kill(pid2, SIGUSR2); // inform child2
  } else { // children
    if (pid1 == 0) { // child 1
      kill(ppid, SIGUSR1);
    } else if (pid2 == 0) { // child 2
      kill(ppid, SIGUSR2);
    }
    sigsuspend(&empty_set); // unmask all signals and wait

  }

  calculate2();

  return 0;
}

void handler(int signal) { }

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

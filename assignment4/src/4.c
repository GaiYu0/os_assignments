#include<signal.h>
#include<stdbool.h>
#include<sys/types.h>
#include<unistd.h>

bool _finished1 = false;
bool _finished2 = false;
bool _continue = false;

void handler(int);
void calculate1();
void calculate2();

pid_t pid;

int main() {
  pid_t parent_pid, pid1, pid2;
  parent_pid = getpid(); // parent pid

  // mask
  sigset_t mask;
  sigfillset(&mask);

  struct sigaction action;
  action.sa_mask = mask;
  action.sa_handler = &handler;
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);

  pid = fork(); // fork child 1
  pid1 = pid;
  if (pid != 0) {
    pid = fork(); // fork child 2
    pid2 = pid;
  }

  calculate1();

  if (pid) {
    // parent
    while(!(_finished1 && _finished2)); // block
    // inform children to continue
    kill(pid1, SIGUSR1);
    kill(pid2, SIGUSR2);
  } else {
    // children
    // inform parent
    if (pid1 == 0) {
      kill(parent_pid, SIGUSR1);
    } else if (pid2 == 0) {
      kill(parent_pid, SIGUSR2);
    }
    while (!_continue); // block;
  }
  calculate2();

  return 0;
}

void handler(int signal) {
  if (pid) {
    // parent
    switch (signal) {
    case SIGUSR1:
      _finished1 = true;
      break;
    case SIGUSR2:
      _finished2 = true;
      break;
    }
  } else {
    // child
    _continue = true;
  }
}

void calculate1() {
  int i;
  for (i = 0; i != 1E8; i++);
}

void calculate2() {
  int i;
  for (i = 0; i != 1E8; i++);
}

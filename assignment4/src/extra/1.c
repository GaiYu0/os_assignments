#define _GNU_SOURCE

#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

int _signal_info;

void status_retrieving_wait_handler(int signal, siginfo_t *info, void *pointer) {
  if (signal == SIGCHLD) {
    _signal_info = *info;
  }
}

// an implementation capable of retrieving a child's termination status
// Question 1.1
int status_retrieving_wait(int *status) {
  struct sigaction action, previous_action;
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = &status_retrieving_wait_handler;

  sigaction(SIGCHLD, &action, &previous_action);

  pause();

  sigaction(SIGCHLD, &previous_action, NULL); // restore previous action

  if (status) {
    *status = _signal_info.sig_status;
  }

  return _signal_info.si_pid;
}

// an implementation for Question 1.2
// n_wait uses pipe to acquire information from ps
#define MAXIMUM_ZOMBIES 16
int n_wait() {
  // an array containing the pid of zombies
  static pid_t zombie_array[MAXIMUM_ZOMBIES];
  static int zombie_array_length = 0;

  // acquire information through ps
  pid_t ppid;
  ppid = getpid();
  char *command;
  asprintf(&command, "ps --ppid %d -o pid,stat", ppid); 
  FILE *pipe;
  pipe = popen(command, "r");

  char *line = NULL;
  int line_length = 0;
  int match;
  pid_t pid;
  char state[3];
  while (1) { // busy waiting
    while (getline(&line, &line_length, pipe) != -1) { // read a line from pipe
      match = sscanf(line, "%d %s", &pid, &state); // search for pid and state
      if (match == 2) {
        if (state[0] == 'Z') { // the process is a zombie
          int index;
          for (index = 0; index != zombie_array_length; index++) { // check whether the zombie is in the array
            if (zombie_array[index] == pid) {
              break;
            }
          }
          if (index == zombie_array_length) { // pid not in the array
            if (zombie_array_length == MAXIMUM_ZOMBIES) { // overflow
              exit(256);
            }
            zombie_array[index] = pid; // insert the process to zombie array
            zombie_array_length++;
            return pid; // end of waiting
          }
        }
      }
    }
  }
  return 0;
}

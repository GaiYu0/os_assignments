#include<signal.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

int _status;
pid_t _terminated_pid;

void _wait_handler(int signal, siginfo_t *info, void *pointer) {
  if (signal == SIGCHLD) {
    _status = info->si_status;
    _terminated_pid = info->si_pid;
  }
}

int mywait(int *status) {
  sigset_t signal_set;
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGCHLD);

  sigset_t signal_mask;
  sigfillset(&signal_mask);
  sigdelset(&signal_mask, SIGCHLD);

  struct sigaction action;
  action.sa_flags = SA_SIGINFO;
  action.sa_mask = signal_mask;
  action.sa_sigaction = &_wait_handler;

  sigaction(SIGCHLD, &action, NULL);

  pause();

  if (status) {
    *status = _status;
  }
  return _terminated_pid;
}

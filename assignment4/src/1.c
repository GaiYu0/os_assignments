#include<signal.h>
#include<unistd.h>

#include<header.h>

struct sigaction _previous_actions[NSIG - 2]; // cache previous signal action

int _remaining_time = 0;

void _handler_alrm(int signal) {
  if (signal == SIGALRM) { // sleeping time elapsed
    _remaining_time = 0;
  }
  else { // sleeping interrupted
    (*(_previous_actions[signal - 2].sa_handler))(signal); // call user handler
    _remaining_time = alarm(0); // remaining sleeping time
  }
}

int mysleep(int seconds) {
  sigset_t mask, previous_mask;
  sigfillset(&mask);
  sigprocmask(SIG_SETMASK, &mask, &previous_mask); // mask all signals

  struct sigaction action;
  action.sa_handler = &_handler_alrm;
  action.sa_mask = mask;

  int i;
  for (i = 2; i != NSIG; i++) // register handlers and cache previous action vectors
    sigaction(i, &action, &_previous_actions[i - 2]);

  alarm(seconds);

  sigsuspend(&previous_mask); // use the mask of user

  for (i = 2; i != NSIG; i++) // register handlers and cache previous action vectors
    sigaction(i, &action, &_previous_actions[i - 1]);

  sigprocmask(SIG_SETMASK, &previous_mask, NULL); // reset mask

  return _remaining_time;
}

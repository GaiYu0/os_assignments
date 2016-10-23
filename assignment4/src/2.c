#include<signal.h>
#include<unistd.h>

#include<header.h>

struct sigaction _previous_actions[NSIG - 1]; // cache previous signal action

int _remaining_time = 0;

void _handler_alrm(int signal) {
  if (signal != SIGALRM) {
    _remaining_time = alarm(0); // remaining sleeping time
  } else {
    _remaining_time = 0; // end of sleep
  }
  // restore handler vector
  int i;
  for (i = 1; i != NSIG; i++) {
    sigaction(i, &_previous_actions[i - 1], NULL);
  }
}

int mysleep(int seconds) {
  sigset_t signal_mask, previous_mask;
  sigfillset(&signal_mask); // mask

  // mask all interruptions
  sigprocmask(SIG_SETMASK, &signal_mask, &previous_mask);

  // initialize signal action
  struct sigaction action;
  action.sa_handler = &_handler_alrm;
  action.sa_mask = signal_mask;

  // register signal action
  int i;
  for (i = 2; i != NSIG; i++) {
    sigaction(i, &action, &_previous_actions[i - 1]); // cache previous action vector
  }

  sigprocmask(SIG_UNBLOCK, &signal_mask, &previous_mask); // unmask

  alarm(seconds);

  pause();
 
  return _remaining_time;
}

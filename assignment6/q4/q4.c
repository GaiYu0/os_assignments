#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>

int N; // the total number of secondary threads that should be created

// indicates the total number of non-terminated secondary threads
int count = 0;
pthread_mutex_t mcount = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t minterrupt = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cinterrupt = PTHREAD_COND_INITIALIZER;

pthread_cond_t ccreation = PTHREAD_COND_INITIALIZER;
pthread_cond_t ctermination = PTHREAD_COND_INITIALIZER;

void handler(int signal);
void *thread(void*);

int main(int argc, char *argv[]) {
  // mask interrupts
  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  // setup handler
  struct sigaction action;
  action.sa_handler = &handler;
  sigaction(SIGINT, &action, NULL);

  // N secondary threads
  N = atoi(argv[1]);

  // initiate creation
  pthread_t tid;
  count += 1; // there is only one thread currently and locking is therefore unnecessary
  pthread_create(&tid, NULL, &thread, NULL);

  pthread_mutex_lock(&mcount);
  if (count != N) { // wait for the end of creation
    pthread_cond_wait(&ccreation, &mcount);
  }
  pthread_mutex_unlock(&mcount);

  // logging
  printf("all descendants created\n");
  printf("please signal me\n");

  // wait for signal
  sigemptyset(&mask);
  sigsuspend(&mask);

  // awaken every secondary thread 
  pthread_mutex_lock(&minterrupt);
  pthread_cond_broadcast(&cinterrupt);
  pthread_mutex_unlock(&minterrupt);

  pthread_mutex_lock(&mcount);
  if (count != 0) { // wait for every secondary thread to terminate
    pthread_cond_wait(&ctermination, &mcount);
  }
  pthread_mutex_unlock(&mcount);
  
  printf("all descendants terminated\n"); // logging
  return 0;
}

void handler(int signal) { // handles SIGINT
  return;  
}

void *thread(void *pointer) { // secondary threads
  printf("thread created\n"); // logging

  pthread_mutex_lock(&mcount);
  if (count != N) { // create another thread
    pthread_t tid;
    pthread_create(&tid, NULL, &thread, NULL);
    count++;
  } else { // every secondary thread created, signal main thread
    pthread_cond_signal(&ccreation);
  }
  pthread_mutex_unlock(&mcount);

  // wait to be awakened by main thread
  pthread_mutex_lock(&minterrupt);
  pthread_cond_wait(&cinterrupt, &minterrupt);
  pthread_mutex_unlock(&minterrupt);

  // terminate
  pthread_mutex_lock(&mcount);
  count--;
  printf("thread terminated\n"); // logging
  if (count == 0) { // every secondary thread terminated, signal main thread
    pthread_cond_signal(&ctermination);
  }
  pthread_mutex_unlock(&mcount);
  return NULL;
}

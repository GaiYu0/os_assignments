#include<fcntl.h>
#include<pthread.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>

#include<header.h>

#define MAXIMUM_N_FILES 4096
#define NORMAL

array_t global_file_locks;
pthread_mutex_t array_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct file_lock {
  char *path;
  int reference_counter;
  pthread_mutex_t counter_lock;
  pthread_cond_t condition;
} file_lock_t;

int search_file_lock(char *path) {
  int i;
  for (i = 0; i != global_file_locks.length; i++)
    if (strcmp(((file_lock_t*)(global_file_locks.array[i]))->path, path) == 0) return i;
  return -1;
}

int construct_global_file_lock() {
  construct_array(&global_file_locks);
  return 0;
}

int global_file_lock_gc() {
  if (global_file_locks.length > MAXIMUM_N_FILES) {
    int i;
    for (i = 0; i != global_file_locks.length; i++) {
      pthread_mutex_lock(&(GET(file_lock_t, global_file_locks, i).counter_lock));
      if (GET(file_lock_t, global_file_locks, i).reference_counter == 0) {
        free(global_file_locks.array[i]);
        array_delete(&global_file_locks, i);
      }
      pthread_mutex_unlock(&(GET(file_lock_t, global_file_locks, i).counter_lock));
    }
  }
  return 0;
}

int construct_file_lock(char *path) {
  global_file_lock_gc();
  // pthread_mutex_lock(&array_lock);
  if (search_file_lock(path) != -1) { LOG_ERROR(); return -1; }
  file_lock_t lock;
  lock.path = path;
  lock.reference_counter = 0;
  pthread_mutex_init(&(lock.counter_lock), NULL);
  pthread_cond_init(&(lock.condition), NULL);
  APPEND(file_lock_t, global_file_locks, lock);
  // pthread_mutex_unlock(&array_lock);
  return 0;
}

#ifdef NORMAL
int ropen(char *path, int flags, mode_t mode) {
  pthread_mutex_lock(&array_lock);
  int index;
  if ((index = search_file_lock(path)) == -1) { // TODO
    if (construct_file_lock(path) == -1) { LOG_ERROR(); pthread_mutex_unlock(&array_lock); return -1; }
    index = global_file_locks.length - 1;
  }
  file_lock_t *lock = (file_lock_t*)(global_file_locks.array[index]);
  pthread_mutex_unlock(&array_lock);
  pthread_mutex_lock(&(lock->counter_lock));
  lock->reference_counter++;
  pthread_mutex_unlock(&(lock->counter_lock));
  return open(path, flags, mode);
}
#else
int ropen(char *path, int flags, mode_t mode) {
  return open(path, flags, mode);
}
#endif

#ifdef NORMAL
int rclose(char *path) {
  pthread_mutex_lock(&array_lock);
  int index;
  if ((index = search_file_lock(path)) == -1) { LOG_ERROR(); pthread_mutex_unlock(&array_lock); return -1; }
  file_lock_t *lock = (file_lock_t*)(global_file_locks.array[index]);
  pthread_mutex_unlock(&array_lock);
  pthread_mutex_lock(&(lock->counter_lock));
  lock->reference_counter--;
  if (lock->reference_counter == 0) { pthread_cond_broadcast(&(lock->condition)); }
  pthread_mutex_unlock(&(lock->counter_lock));
  return 0;
}
#else
int rclose(char *path) {
  return 0;
}
#endif

#ifdef NORMAL
int wopen(char *path, int flags, mode_t mode) {
  pthread_mutex_lock(&array_lock);
  int index;
  if ((index = search_file_lock(path)) == -1) { // TODO
    if (construct_file_lock(path) == -1) { LOG_ERROR(); pthread_mutex_unlock(&array_lock); return -1; }
    index = global_file_locks.length - 1;
  }
  file_lock_t *lock = (file_lock_t*)(global_file_locks.array[index]);
  pthread_mutex_unlock(&array_lock);
  pthread_mutex_lock(&(lock->counter_lock));
  while (lock->reference_counter > 0) pthread_cond_wait(&(lock->condition), &(lock->counter_lock));
  return open(path, flags, mode);
}
#else
int wopen(char *path, int flags, mode_t mode) {
  return open(path, flags, mode);
}
#endif

#ifdef NORMAL
int wclose(char *path) {
  pthread_mutex_lock(&array_lock);
  int index;
  if ((index = search_file_lock(path)) == -1) { LOG_ERROR(); pthread_mutex_unlock(&array_lock); return -1; } 
  file_lock_t *lock = (file_lock_t*)(global_file_locks.array[index]);
  pthread_mutex_unlock(&array_lock);
  pthread_mutex_unlock(&(lock->counter_lock));
  return 0;
}
#else
int wclose(char *path) {
  return 0;
}
#endif

int destroy_global_file_lock() {
  int i;
  for (i = 0; i != global_file_locks.length; i++)
    free(global_file_locks.array[i]);
  destroy_array(&global_file_locks);
  return 0;
}

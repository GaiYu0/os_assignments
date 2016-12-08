#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>

#include<header.h>

#define MAXIMUM_WAITING_TIME 3

int _send_to(int fd, void *buffer, size_t size) {
  size_t offset = 0;
  size_t length;
  while (offset != size) {
    if ((length = write(fd, buffer + offset, size)) == -1) { PERROR("write"); return -1; }
    offset += length;
  }
  return 0;
}

int send_to(int fd, void *buffer, size_t size) {
  if (_send_to(fd, &size, sizeof(size_t)) == -1) { LOG_ERROR(); return -1; }
  if (_send_to(fd, buffer, size) == -1) { LOG_ERROR(); return -1; }
  return 0;
}

int _receive_from(int fd, void *buffer, size_t size) {
  float waiting_time = 0.1;
  int offset = 0;
  int length;
  while (offset != size) {
    if ((length = read(fd, buffer + offset, size)) == -1) { PERROR("read"); return -1; }
    offset += length;
    if (length == 0) {
      waiting_time *= 2;
      if (waiting_time > MAXIMUM_WAITING_TIME) {
        return -1;
      }
      sleep(waiting_time);
    }
  }
  return 0;
}

int receive_from(int fd, void **buffer) {
  size_t size;
  if (_receive_from(fd, &size, sizeof(size_t)) == -1) { LOG_ERROR(); return -1; }
  int allocated = 0;
  if (*buffer == NULL) { *buffer = malloc(size); allocated = 1; }
  if ((_receive_from(fd, *buffer, size) == -1)) {
    LOG_ERROR();
    if (allocated) { free(*buffer); }
    return -1;
  }
  return size;
}

char *join_strings(char **strings, char *delimiter) {
  int delimiter_length;
  delimiter_length = strlen(delimiter);
  int total_length = 0;
  int count;
  for (count = 0; strings[count] != NULL; count++) {
    total_length += strlen(strings[count]) + delimiter_length;
  }
  total_length -= delimiter_length;
  char *string;
  string = (char*)calloc(total_length + 1, sizeof(char));
  int i;
  for (i = 0; i != count - 1; i++) {
    strcat(string, strings[i]);
    strcat(string, delimiter);
  }
  strcat(string, strings[count - 1]);
  return string;
}

int send_file(int socket, int fd) {
#define FUNCTION SEND_FILE
  int returned_value;

  struct stat file_status;
  fstat(fd, &file_status);
  if (send_to(socket, &(file_status.st_size), sizeof(off_t)) == -1) { LOG_ERROR(); RETURN(-1); }

  void *buffer;
  buffer = malloc(BUFFER_SIZE);
  size_t length;
  while (1) {
    length = read(fd, buffer, BUFFER_SIZE);
    if (length == 0) { break; } else if (length == -1) { LOG_ERROR(); RETURN(-1); }
    if (send_to(socket, buffer, length) == -1) { LOG_ERROR(); RETURN(-1); }
  }

  RETURN(0);

#undef FUNCTION

FINALIZE_SEND_FILE:
  FREE(buffer);
  return returned_value;
}

int receive_file(int socket, int fd, int flags) {
#define FUNCTION RECEIVE_FILE
  int returned_value;

  off_t file_size, *_file_size;
  _file_size = &file_size;
  if (receive_from(socket, (void**)&_file_size) == -1) { LOG_ERROR(); RETURN(-1); }
  if (flags & O_TRUNC) {
    if (ftruncate(fd, file_size) == -1) { PERROR("ftruncate"); RETURN(-1); }
  }

  size_t length;
  void *buffer = malloc(BUFFER_SIZE);
  while (file_size != 0) {
    if ((length = receive_from(socket, &buffer)) == -1) { LOG_ERROR(); RETURN(-1); }
    if (write(fd, buffer, length) == -1) { LOG_ERROR(); RETURN(-1); }
    file_size -= length;
  }

  RETURN(0);

#undef FUNCTION

FINALIZE_RECEIVE_FILE:
  FREE(buffer);
  return returned_value;
}

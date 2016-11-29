#include<errno.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

#define BUFFER_LENGTH 16
#define FOLDER "storage"

int socket_connection, socket_client;
struct sockaddr_in address_server, address_client;
void cleanup();

int main(int argc, char *argv[]) {
  atexit(&cleanup);
  if ((socket_connection = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(0);
  }
  memset(&address_server, 0, sizeof(struct sockaddr_in));
  address_server.sin_addr.s_addr = htonl(INADDR_ANY);
  address_server.sin_family = AF_INET;
  if (argc == 2) {
    address_server.sin_port = htons(atoi(argv[1]));
  } else {
    address_server.sin_port = htons(SERVER_PORT);
  }

  int indicator;
  indicator = bind(
    socket_connection,
    (struct sockaddr*)(&address_server),
    (socklen_t)sizeof(struct sockaddr_in)
  );
  if (indicator == -1) {
    perror("bind");
    exit(0);
  }

  socklen_t address_length = sizeof(struct sockaddr_in);
  if (listen(socket_connection, 4) == -1) {
    perror("listen");
    exit(0);
  }
  socket_client = accept(socket_connection, (struct sockaddr*)(&address_client), &address_length);
  if (socket_client == -1) {
    perror("accept");
    exit(0);
  }

  struct stat status;
  if ((stat(FOLDER, &status) != 0) || (!S_ISDIR(status.st_mode))) {
    if (mkdir(FOLDER, S_IRWXU | S_IRWXG | S_IXOTH) == -1) {
      perror("mkdir");
      exit(0);
    }
  }
  char file_name[MAXIMUM_FILE_NAME_LENGTH + 1];
  if (read(socket_client, file_name, MAXIMUM_FILE_NAME_LENGTH + 1) == -1) {
    perror("read");
    exit(0);
  }
  char *path;
  asprintf(&path, "%s/%s", FOLDER, file_name);
  printf("%s\n", path);
  int fd;
  if ((fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0600)) == -1) {
    if (errno == EEXIST) {
      // TODO feedback
    } else {
      perror("open");
      exit(0);
    }
  }
  char buffer[BUFFER_LENGTH];
  int length = 1;
  while(length != 0) {
    if ((length = read(socket_client, buffer, BUFFER_LENGTH * sizeof(char))) == -1) {
      perror("read");
      exit(0);
    }
    write(fd, buffer, length * sizeof(char));
  }
  cleanup();
  return 0;
}

void cleanup() {
  shutdown(socket_client, SHUT_RDWR);
  close(socket_client);
}

#include<arpa/inet.h>
#include<fcntl.h>
#include<libgen.h>
#include<netdb.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

#define BUFFER_SIZE 1024
#define MESSAGE "message"

int socket_client;
void cleanup();

int main(int argc, char *argv[]) {
  atexit(&cleanup);
  if (argc != 4) {
    printf("Invalid arguments!\n");
  }
  if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(0);
  }

  struct sockaddr_in address_server;
  memset(&address_server, 0, sizeof(struct sockaddr_in));
  if (strcmp(argv[1], "default") == 0) {
    address_server.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    if (inet_aton(argv[1], &(address_server.sin_addr)) == 0) {
      perror("inet_aton");
      exit(0);
    }
  }
  address_server.sin_family = AF_INET;
  if (strcmp(argv[2], "default") == 0) {
    address_server.sin_port = htons(SERVER_PORT);
  } else {
    address_server.sin_port = htons((short)atoi(argv[2]));
  }

  int indicator;
  indicator = connect(
    socket_client,
    (struct sockaddr*)(&address_server),
    (socklen_t)sizeof(struct sockaddr_in)
  );
  if (indicator == -1) {
    perror("connect");
    exit(0);
  }

  int fd;
  if ((fd = open(argv[3], O_RDONLY)) == -1) {
    perror("open");
    exit(0);
  }
  struct stat status;
  fstat(fd, &status);
  if (!S_ISREG(status.st_mode)) {
    printf("Files to be uploaded must be regular files.\n");
    exit(0);
  }
  char cache[MAXIMUM_FILE_NAME_LENGTH + 1], *file_name;
  strcpy(cache, argv[3]);
  file_name = basename(cache);
  if (strlen(file_name) > MAXIMUM_FILE_NAME_LENGTH) {
    printf("Unsupported file name!\n");
    exit(0);
  }
  strcpy(cache, file_name);
  write(socket_client, cache, (MAXIMUM_FILE_NAME_LENGTH + 1) * sizeof(char));
  char buffer[BUFFER_SIZE];
  int length = 1;
  while (length != 0) {
    length = read(fd, buffer, BUFFER_SIZE * sizeof(char));
    write(socket_client, buffer, length * sizeof(char));
  }

  cleanup();
  return 0;
}

void cleanup() {
  shutdown(socket_client, SHUT_RDWR);
  close(socket_client);
}

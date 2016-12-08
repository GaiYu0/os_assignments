#include<arpa/inet.h>
#include<fcntl.h>
#include<libgen.h>
#include<netdb.h>
#include<netinet/in.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/file.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

#define C_UPLOAD "upload"
#define C_DOWNLOAD "download"
#define C_EXECUTE "execute"

int CLIENT_ERROR;

int socket_client;
int client_fd;
int file_locked;

int cleaned;
void cleanup();
void signal_cleanup(int signal);

int upload(int argc, char *argv[]);
int download(int argc, char *argv[]);
int execute(int argc, char *argv[]);

int initialize_client(int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = &signal_cleanup;
  sigaction(SIGINT, &action, NULL);
  atexit(&cleanup);
  return 0;
}

int connect_to_server(int argc, char *argv[], struct sockaddr_in *address_server) {
  if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) == -1) { PERROR("socket"); return -1; }

  memset(address_server, 0, sizeof(struct sockaddr_in));
  if (strcmp(argv[1], "default") == 0) { address_server->sin_addr.s_addr = htonl(INADDR_ANY); }
  else { inet_aton(argv[1], &(address_server->sin_addr)); }
  address_server->sin_family = AF_INET;
  if (strcmp(argv[2], "default") == 0) { address_server->sin_port = htons(SERVER_PORT);
  } else { address_server->sin_port = htons((short)atoi(argv[2])); }

  if (connect(socket_client, (struct sockaddr*)address_server, (socklen_t)sizeof(struct sockaddr_in)) == -1) {
    PERROR("connect");
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 5) { printf("unrecongnized command\n"); exit(0); }
  initialize_client(argc, argv);
  struct sockaddr_in address_server;
  if (connect_to_server(argc, argv, &address_server) == 0) {
    printf(
      "connected to server %s:%d\n",
      inet_ntoa(address_server.sin_addr),
      address_server.sin_port
    );
  } else { printf("cannot connect to server\n"); exit(0); }
  int status;
  if (strcmp(argv[3], C_UPLOAD) == 0) { status = upload(argc, argv); }
  else if (strcmp(argv[3], C_DOWNLOAD) == 0) { status = download(argc, argv); }
  else if (strcmp(argv[3], C_EXECUTE) == 0) { status = execute(argc, argv); }
  else { printf("unrecongnized command\n"); exit(0); }

  int i;
  for (i = 3; i != argc - 1; i++) { printf("%s ", argv[i]); }
  printf("%s\n", argv[argc - 1]);

  if (status != 0) { printf("error\n"); }

  return 0;
}

int upload(int argc, char *argv[]) {
  request_t request = S_UPLOAD;
  if (send_to(socket_client, &request, sizeof(request_t)) == -1) { LOG_ERROR(); return -1; }

  if ((client_fd = open(argv[4], O_RDONLY)) == -1) { PERROR("open"); return -1; }

  if (flock(client_fd, LOCK_EX) == -1) { PERROR("flock"); return -1; }
  file_locked = 1;

  size_t path_size;
  path_size = (strlen(argv[5]) + 1) * sizeof(char);
  if (send_to(socket_client, argv[5], path_size) == -1) { LOG_ERROR(); return -1; }

  void *buffer;
  buffer = malloc(BUFFER_SIZE);

  struct stat file_status;
  fstat(client_fd, &file_status);
  if (send_file(socket_client, client_fd) == -1) { LOG_ERROR(); return -1; }

  status_t server_status, *_server_status;
  _server_status = &server_status;
  if (receive_from(socket_client, (void**)&_server_status) == -1) { LOG_ERROR(); return -1; }
  if (server_status == S_SUCCESS) { return 0; }
  else { return -1; }
}

int download(int argc, char *argv[]) {
  request_t request = S_DOWNLOAD;
  if (send_to(socket_client, &request, sizeof(request_t)) == -1) { LOG_ERROR(); return -1; }

  size_t path_size;
  path_size = (strlen(argv[4]) + 1) * sizeof(char);
  if (send_to(socket_client, argv[4], path_size) == -1) { LOG_ERROR(); return -1; }
  if ((client_fd = open(argv[5], O_WRONLY, S_IWUSR)) == -1) { LOG_ERROR(); return -1; }
  if (receive_file(socket_client, client_fd, O_TRUNC) == -1) { LOG_ERROR(); return -1; }

  return 0;
}

int execute(int argc, char *argv[]) {
  request_t request = S_EXECUTE;
  if (send_to(socket_client, &request, sizeof(request_t)) == -1) { LOG_ERROR(); return -1; }
  
  int count;
  count = argc - 4;
  if (send_to(socket_client, &count, sizeof(int)) == -1) { LOG_ERROR(); return -1; }
  int i;
  size_t size;
  for (i = 4; i != argc; i++) {
    size = (strlen(argv[i]) + 1) * sizeof(char);
    if (send_to(socket_client, argv[i], size) == -1) { LOG_ERROR(); return -1; }
  }

  if (receive_file(socket_client, STDOUT_FILENO, 0) == -1) { LOG_ERROR(); return -1; }

  status_t server_status, *_server_status;
  _server_status = &server_status;
  if (receive_from(socket_client, (void**)&_server_status) == -1) { LOG_ERROR(); return -1; }
  if (server_status == S_SUCCESS) { return 0; }
  else { printf("%d\n", server_status); return -1; }

  return 0;
}

void cleanup() {
  if (!cleaned) {
    if (socket_client > 0) {
      shutdown(socket_client, SHUT_RDWR);
      close(socket_client);
    }
    if (file_locked) { flock(client_fd, LOCK_UN); }
    if (client_fd > 0) {
      close(client_fd);
    }
  }
}

void signal_cleanup(int signal) {
  cleanup();
}

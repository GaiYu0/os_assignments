#include<arpa/inet.h>
#include<dirent.h>
#include<fcntl.h>
#include<libgen.h>
#include<netdb.h>
#include<netinet/in.h>
#include<pthread.h>
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
struct sockaddr_in address_server;

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

int connect_to_server(int argc, char *argv[]) {
  if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) == -1) { PERROR("socket"); return -1; }

  memset(&address_server, 0, sizeof(struct sockaddr_in));
  if (strcmp(argv[1], "default") == 0) { address_server.sin_addr.s_addr = htonl(INADDR_ANY); }
  else { inet_aton(argv[1], &(address_server.sin_addr)); }
  address_server.sin_family = AF_INET;
  if (strcmp(argv[2], "default") == 0) { address_server.sin_port = htons(SERVER_PORT);
  } else { address_server.sin_port = htons((short)atoi(argv[2])); }

  if (
    connect(socket_client, (struct sockaddr*)&address_server, (socklen_t)sizeof(struct sockaddr_in)) == -1
  ) { PERROR("connect"); return -1; }

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 5) { printf("unrecongnized command\n"); exit(0); }
  initialize_client(argc, argv);
  if (connect_to_server(argc, argv) == 0) {
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

typedef struct uploading_session {
  int fd, socket;
  char *path;
  pthread_t tid;
} uploading_session_t;

uploading_session_t *uploading_sessions = NULL;

void *_upload(void *pointer) {
  uploading_session_t *session = (uploading_session_t*)pointer;
  request_t request = S_UPLOAD;
  if (send_to(session->socket, &request, sizeof(request_t)) == -1) { LOG_ERROR(); return (void*)1; }
  size_t path_size = (strlen(session->path) + 1) * sizeof(char);
  if (send_to(session->socket, session->path, path_size) == -1) { LOG_ERROR(); return (void*)1; }
  if (send_file(session->socket, session->fd) == -1) { LOG_ERROR(); return (void*)1; }
  return 0;
}

int upload(int argc, char *argv[]) {
#define FUNCTION UPLOAD
  int returned_value;

  int n_files = 0;
  char **files = NULL;
  char *file_name = NULL;
  int fd = 0;
  int i = 0;
  if ((fd = open(argv[4], O_RDONLY)) == -1) { PERROR("open"); RETURN(-1); }
  struct stat file_status;
  if (fstat(fd, &file_status) == -1) { PERROR("fstat"); RETURN(-1); }
  if (S_ISDIR(file_status.st_mode)) {
    DIR *directory;
    if ((directory = opendir(argv[4])) == NULL) { PERROR("opendir"); RETURN(-1); }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
      if (entry->d_type == DT_REG) { n_files++; }
    if (n_files == 0) { RETURN(0); }
    files = (char**)calloc(n_files, sizeof(char*));
    rewinddir(directory);
    while ((entry = readdir(directory)) != NULL)
      if (entry->d_type == DT_REG) { asprintf(files + (i++), "%s", entry->d_name); }
  } else { n_files = 1; asprintf(files, "%s", argv[4]); }

  uploading_sessions = (uploading_session_t*)malloc(n_files * sizeof(uploading_session_t));
  for (i = 0; i != n_files; i++) {
    uploading_sessions[i].path = files[i];
    FREE(file_name);
    asprintf(&file_name, "%s/%s", argv[4], files[i]);
    if ((uploading_sessions[i].fd = open(file_name, O_RDONLY)) == -1) { PERROR("open"); RETURN(-1); }
    if ((uploading_sessions[i].socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { PERROR("socket"); return -1; }
    if (
      connect(
        uploading_sessions[i].socket,
        (struct sockaddr*)&address_server,
        (socklen_t)sizeof(struct sockaddr_in)
      ) == -1
    ) { PERROR("connect"); RETURN(-1); }
  }

  for (i = 0; i != n_files; i++) {
    if (
      pthread_create(&(uploading_sessions[i].tid), NULL, &_upload, uploading_sessions + i) != 0
    ) { LOG_ERROR(); RETURN(-1); }
  }

  long status, global_status;
  for (i = 0, global_status = 1; i != n_files; i++) {
    pthread_join(uploading_sessions[i].tid, (void**)&status);
    if (status == 1) { global_status = 0; }
  }

  if (global_status) { RETURN(0); }
  else { RETURN(-1); }

#undef FUNCTION
FINALIZE_UPLOAD:
  FREE(file_name);
  if (fd > 0) close(fd);
  if (files != NULL)
    for (i = 0; i != n_files; i++) {
      if (files[i] != NULL) free(files[i]);
      else break;
    }
  FREE(files);
  if (uploading_sessions != NULL)
    for (i = 0; i != n_files; i++) {
      if (uploading_sessions[i].socket > 0) {
        shutdown(uploading_sessions[i].socket, SHUT_RDWR);
        close(uploading_sessions[i].socket);
      }
      if (uploading_sessions[i].fd > 0) { close(uploading_sessions[i].fd); }
    }
  FREE(uploading_sessions);
  return returned_value;
}
 
typedef struct downloading_session {
  int fd, socket;
  char *path;
  pthread_t tid;
} downloading_session_t;

downloading_session_t *downloading_sessions = NULL;

void *_download(void *pointer) {
  downloading_session_t *session = (downloading_session_t*)pointer;
  request_t request = S_DOWNLOAD;
  if (send_to(session->socket, &request, sizeof(request_t)) == -1) { LOG_ERROR(); return (void*)1; }
  size_t path_size = (strlen(session->path) + 1) * sizeof(char);
  if (send_to(session->socket, session->path, path_size) == -1) { LOG_ERROR(); return (void*)1; }
  int n_files;
  if (receive_from(session->socket, (void**)&n_files) == -1) { LOG_ERROR(); return (void*)1; }
  if (n_files != -1) { LOG_ERROR(); return (void*)1; }
  if (receive_file(session->socket, session->fd, O_TRUNC) == -1) { LOG_ERROR(); return (void*)1; }
  return 0;
}

int download(int argc, char *argv[]) {
#define FUNCTION UPLOAD
  int returned_value;

  int n_files = 0;
  int *_n_files = &n_files;
  char **files = NULL;
  char *file_name = NULL;
  int fd = 0;
  int i = 0; 

  size_t path_size = (strlen(argv[4]) + 1) * sizeof(char);
  if (send_to(socket_client, argv[4], path_size) == -1) { LOG_ERROR(); RETURN(-1); }
  if (receive_from(socket_client, (void**)&_n_files) == -1) { LOG_ERROR(); RETURN(-1); }
  switch (n_files) {
    case -1:
      if ((fd = open(argv[4], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1) { PERROR("open"); RETURN(-1); }
      if (receive_file(socket_client, fd, O_TRUNC) == -1) { LOG_ERROR(); RETURN(-1); }
      RETURN(0);
    case 0:
      RETURN(0);
    default:
      files = (char**)calloc(n_files, sizeof(char*));
      downloading_sessions = (downloading_session_t*)malloc(n_files * sizeof(downloading_session_t));
      for (i = 0; i != n_files; i++) {
        if (receive_from(socket_client, (void**)(files + i)) == -1) { LOG_ERROR(); RETURN(-1); }
        asprintf(&(downloading_sessions[i].path), "%s/%s", argv[4], files[i]);
        if (
          (downloading_sessions[i].fd = open(files[i], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR)) == -1
        ) { PERROR("open"); RETURN(-1); }
        if ((downloading_sessions[i].socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) { PERROR("socket"); return -1; }
        if (
          connect(
            downloading_sessions[i].socket,
            (struct sockaddr*)&address_server,
            (socklen_t)sizeof(struct sockaddr_in)
          ) == -1
        ) { PERROR("connect"); RETURN(-1); }
      }

      for (i = 0; i != n_files; i++) {
        if (
          pthread_create(&(downloading_sessions[i].tid), NULL, &_download, downloading_sessions + i) != 0
        ) { LOG_ERROR(); RETURN(-1); }
      }

      long status, global_status;
      for (i = 0, global_status = 1; i != n_files; i++) {
        pthread_join(downloading_sessions[i].tid, (void**)&status);
        if (status == 1) { global_status = 0; }
      }

      if (global_status) { RETURN(0); }
      else { RETURN(-1); }
  }

#undef FUNCTION
FINALIZE_UPLOAD:
  FREE(file_name);
  if (fd > 0) close(fd);
  if (files != NULL)
    for (i = 0; i != n_files; i++) {
      if (files[i] != NULL) free(files[i]);
      else break;
    }
  FREE(files);
  if (downloading_sessions != NULL)
    for (i = 0; i != n_files; i++) {
      if (downloading_sessions[i].socket > 0) {
        shutdown(downloading_sessions[i].socket, SHUT_RDWR);
        close(downloading_sessions[i].socket);
      }
      if (downloading_sessions[i].fd > 0) { close(downloading_sessions[i].fd); }
      FREE(downloading_sessions[i].path);
    }
  FREE(downloading_sessions);
  return returned_value;
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

  return 0;
}

void cleanup() {
  if (!cleaned) {
    if (socket_client > 0) {
      shutdown(socket_client, SHUT_RDWR);
      close(socket_client);
    }
  }
}

void signal_cleanup(int signal) {
  cleanup();
}

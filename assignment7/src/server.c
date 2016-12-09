/*
 * TODO
 * server pressure test
 *
 */

#include<arpa/inet.h>
#include<dirent.h>
#include<errno.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<pthread.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>

#include<header.h>

#define TMP "tmp"
#define STORAGE "file"

char *storage = STORAGE;

typedef struct client_info {
  int socket;
  struct sockaddr_in address;
} client_info_t;

int construct_client_info(client_info_t *client) {
  client->socket = 0;
  memset(&(client->address), 0, sizeof(struct sockaddr_in));
  return 0;
}

int destroy_client_info(client_info_t *client) {
  if (client->socket > 0) {
    shutdown(client->socket, SHUT_RDWR);
    close(client->socket);
    printf(
      "disconnected from client %s:%d\n",
      inet_ntoa(client->address.sin_addr),
      client->address.sin_port
    );
  }
  return 0;
}

int socket_connection;
struct sockaddr_in address_server;
array_t clients;
pthread_mutex_t info_lock = PTHREAD_MUTEX_INITIALIZER;

int cleaned;
void cleanup();
void signal_cleanup(int signal);
void default_handler(int signal);

void* client_thread(void*);

typedef int(*server_function)(client_info_t*);
int upload(client_info_t*);
int download(client_info_t*);
int execute(client_info_t *client);

server_function function_table[N_S_FUNCTIONALITY] = {
  &upload,
  &download,
  &execute,
};

int initialize_server(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  if (argc == 3) { storage = argv[2]; }
  if (initialize_server(argc, argv) == -1) {
    printf("Initialization failed.\n");
    exit(0);
  }
  
  client_info_t *client;
  pthread_t tid;
  while (1) {
    client = (client_info_t*)malloc(sizeof(client_info_t));
    construct_client_info(client);
    socklen_t address_length = sizeof(struct sockaddr_in);
    if (
      (client->socket = accept(socket_connection, (struct sockaddr*)(&(client->address)), &address_length)) == -1
    ) { LOG_ERROR(); continue; }

    pthread_mutex_lock(&info_lock);
    array_append(&clients, (void*)client);
    pthread_create(&tid, NULL, &client_thread, clients.array[clients.length - 1]);
    pthread_mutex_unlock(&info_lock);

    printf(
      "connected to client %s:%d\n",
      inet_ntoa(client->address.sin_addr),
      client->address.sin_port
    );
  }
  return 0;
}

int initialize_server(int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = &signal_cleanup;
  sigaction(SIGINT, &action, NULL);
  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);

  atexit(&cleanup);

  construct_array(&clients);

  if ((socket_connection = socket(AF_INET, SOCK_STREAM, 0)) == -1) { LOG_ERROR(); return -1; }
  memset(&address_server, 0, sizeof(struct sockaddr_in));
  address_server.sin_addr.s_addr = htonl(INADDR_ANY);
  address_server.sin_family = AF_INET;
  if (argc == 2) { address_server.sin_port = htons(atoi(argv[1])); }
  else { address_server.sin_port = htons(SERVER_PORT); }

  int reuse = 1;
  if (setsockopt(socket_connection, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) { LOG_ERROR(); return -1; }

  if (
    bind(socket_connection, (struct sockaddr*)(&address_server), (socklen_t)sizeof(struct sockaddr_in)) == -1
  ) { LOG_ERROR(); return -1; }

  if (listen(socket_connection, 4) == -1) { LOG_ERROR(); return -1; }

  struct stat status;
  if ((stat(TMP, &status) != 0) || (!S_ISDIR(status.st_mode))) {
    if (mkdir(TMP, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) { LOG_ERROR(); return -1; }
  }
  if ((stat(storage, &status) != 0) || (!S_ISDIR(status.st_mode))) {
    if (mkdir(storage, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) { LOG_ERROR(); return -1; }
  }

  return 0;
}

void* client_thread(void* pointer) {
#define FUNCTION CLIENT_THREAD
  void *returned_value;

  client_info_t *client;
  client = (client_info_t*)pointer;

  request_t request, *_request;
  _request = &request;
  if (receive_from(client->socket, (void**)&_request) == -1) { LOG_ERROR(); RETURN(NULL); }
  if ((*(function_table[request]))(client) != 0) { printf("error\n"); }
  RETURN(NULL);

#undef FUNCTION
FINALIZE_CLIENT_THREAD:
  destroy_client_info(client);
  pthread_mutex_lock(&info_lock);
  int index;
  index = array_find_reference(&clients, pointer);
  array_delete(&clients, index); 
  pthread_mutex_unlock(&info_lock);
  return returned_value;
}

int upload(client_info_t *client) {
#define FUNCTION UPLOAD
  int returned_value;

  char *_path = NULL;
  char *path = NULL;
  int fd;
  if (receive_from(client->socket, (void**)&_path) == -1) { LOG_ERROR(); RETURN(-1); }
  asprintf(&path, "%s/%s", storage, _path);
  if ((fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR)) < 0) { PERROR("open"); RETURN(-1); }
  if (receive_file(client->socket, fd, O_TRUNC) == -1) { PERROR("open"); RETURN(-1); }
  
  RETURN(0);

#undef FUNCTION
FINALIZE_UPLOAD:
  if (fd > 0) { close(fd); }
  FREE(path);
  FREE(_path);
  return returned_value;
}

int download(client_info_t *client) {
#define FUNCTION DOWNLOAD
  int returned_value;

  char *_path = NULL;
  char *path = NULL;
  int fd = 0;
  char *entry_path;

  if (receive_from(client->socket, (void**)&_path) == -1) { LOG_ERROR(); RETURN(-1); }
  asprintf(&path, "%s/%s", storage, _path);
  printf("to open %s\n", path);
  if ((fd = open(path, O_RDONLY)) < 0) { PERROR("open"); RETURN(-1); }
  struct stat status;
  if (fstat(fd, &status) == -1) { PERROR("fstat"); RETURN(-1); }
  int n_files = 0;
  if (S_ISDIR(status.st_mode)) {
    DIR *directory;
    if ((directory = opendir(path)) == NULL) { PERROR("opendir"); RETURN(-1); }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
      if (entry->d_type == DT_REG) { n_files++; }
    if (send_to(client->socket, &n_files, sizeof(int)) == -1) { LOG_ERROR(); RETURN(-1); }
    if (n_files == 0) { RETURN(0); }
    rewinddir(directory);
    size_t path_size;
    while ((entry = readdir(directory)) != NULL)
      if (entry->d_type == DT_REG) {
        asprintf(&entry_path, "%s/%s", _path, entry->d_name);
        path_size = (strlen(entry_path) + 1) * sizeof(char);
        if (send_to(client->socket, entry_path, path_size) == -1) { LOG_ERROR(); RETURN(-1); }
      }
  } else {
    n_files = -1;
    if (send_to(client->socket, &n_files, sizeof(int)) == -1) { LOG_ERROR(); RETURN(-1); }
    if (send_file(client->socket, fd) == -1) { PERROR("open"); RETURN(-1); }
  }

  RETURN(0);

#undef FUNCTION
FINALIZE_DOWNLOAD:
  FREE(entry_path);
  if (fd > 0) { close(fd); }
  FREE(path);
  FREE(_path);
  return returned_value;
}

int execute(client_info_t *client) {
#define FUNCTION EXECUTE
  int returned_value;
  char *cache_file = NULL;
  int fd = 0;
  char **arguments = NULL;
  char *_command = NULL;
  char *command = NULL;

  asprintf(&cache_file, "%s/%ld%ld", TMP, getpid(), pthread_self());

  int argument_count = 0;
  int *_argument_count = &argument_count;
  if (receive_from(client->socket, (void**)&_argument_count) == -1) { LOG_ERROR(); RETURN(-1); }
  arguments = (char**)calloc(argument_count + 1, sizeof(char*));
  int i;
  for (i = 0; i != argument_count; i++) {
    if (receive_from(client->socket, (void**)&(arguments[i])) == -1) { LOG_ERROR(); RETURN(-1); }
  }
  _command = join_strings(arguments, " ");
  printf("%s\n", _command);
  asprintf(&command, "(cd %s; %s) > %s", storage, _command, cache_file);
  system(command);
  if ((fd = open(cache_file, O_RDONLY)) == -1) { LOG_ERROR(); RETURN(-1); }
  if (send_file(client->socket, fd) == -1) { LOG_ERROR(); RETURN(-1); }

  RETURN(0);

#undef FUNCTION
FINALIZE_EXECUTE:
  FREE(command);
  FREE(_command);
  if (arguments != NULL) {
    int i;
    for (i = 0; i != argument_count; i++)
      if (arguments[i] == NULL) { break; }
      else { free(arguments[i]); }
  }
  if (fd != 0) { close(fd); remove(cache_file); }
  FREE(cache_file);
  return returned_value;
}

void cleanup() {
  if (!cleaned) {
    shutdown(socket_connection, SHUT_RDWR);
    close(socket_connection);
    pthread_mutex_lock(&info_lock);
    int i;
    for (i = 0; i != clients.length; i++) {
      destroy_client_info(&GET(client_info_t, clients, i));
      free(clients.array[i]);
    }
    destroy_array(&clients);
    pthread_mutex_unlock(&info_lock);
    cleaned = 1;
  }
}

void signal_cleanup(int signal) {
  cleanup();
  exit(0);
}

void default_handler(int signal) {
}

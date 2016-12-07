#include<stdio.h>

// constants
#define BUFFER_SIZE 1024
#define SERVER_PORT 5121
#define MAXIMUM_PATH_SIZE 1024

// server functionality
typedef int request_t;
#define N_S_FUNCTIONALITY 3
#define S_UPLOAD 0
#define S_DOWNLOAD 1
#define S_EXECUTE 2

// server status
typedef int status_t;
#define S_SUCCESS 0
#define S_FAILURE 1
#define S_DUPLICATE_FILE 2
#define S_FILE_WRITING_FAILURE 3
#define S_UPLOADING_INTERRUPTED 4
#define S_INVALID_PATH 5

// client status
#define SERVICE_CANCELLED 0

// dynamic array
typedef struct array_t {
  void **array;
  int maximum_length;
  int length;
} array_t;

int construct_array(array_t*);
int destroy_array(array_t*);
int array_append(array_t*, void*);
void* array_index(array_t*, int);
int array_delete(array_t*, int);
int array_find_reference(array_t*, void*);

#define GET(TYPE, array, index) (*((TYPE*)array_index(&(array), (index))))
#define SET(TYPE, array, index, value) *((TYPE*)array_index(&(array), (index))) = (value)
#define APPEND(TYPE, array, value) \
  TYPE *__pointer__; \
  __pointer__ = (TYPE*)malloc(sizeof(TYPE)); \
  *__pointer__ = (value); \
  int __status__; \
  __status__ = array_append(&(array), __pointer__)

// utilities
int send_to(int fd, void *buffer, size_t size);
int receive_from(int fd, void **buffer);
int send_file(int socket, int fd);
int receive_file(int socket, int fd, int flags);

char *join_strings(char **strings, char *delimiter);

#define TIME_OUT 0

#define FREE(pointer) if ((pointer) != NULL) { free(pointer); }

#define PERROR(message) \
  perror((message)); \
  printf("%s %d: %s\n", __FILE__, __LINE__, __func__)

#define LOG_ERROR() printf("%s %d: %s\n", __FILE__, __LINE__, __func__)
#define MARK() printf("%s %d: %s\n", __FILE__, __LINE__, __func__)

#define FINALIZER(FUNCTION) CONCATENATE(FINALIZE_, FUNCTION)
#define CONCATENATE(left, right) left ## right
#define RETURN(value) returned_value = (value); goto FINALIZER(FUNCTION);

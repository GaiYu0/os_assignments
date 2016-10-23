#define MAX_LINE_SIZE 512
#define MAX_TASK_COUNT 32
#define MAX_TASK_NAME_SIZE 32
#define MAX_QUEUE_COUNT 8

#define UPCOMING 0
#define READY 1
#define RUNNING 2
#define SLEEPING 3
#define TERMINATED 4

typedef struct task {
  /* Initial elements */
  char name[MAX_TASK_NAME_SIZE]; //task name
  int computation_time; //task duration
  int arrival_date; //date of insertion in the system
  /* Used by scheduler */
  int state;
  int execution_time; //nb of cycles on processor
  int remaining_time;
  // time statistics
  int waiting_time;
  int turnaround_time;
  // io
  int io_period;
  int io_duration;
  int sleeping_time;
  int quantum;
} task_t;

typedef struct scheduler_data_t {
  int queue_count;
  int queues[MAX_QUEUE_COUNT][MAX_TASK_COUNT];
  int starts[MAX_QUEUE_COUNT];
  int ends[MAX_QUEUE_COUNT];
  int lengths[MAX_QUEUE_COUNT];
  int quantum;
  int current_priority;
} scheduler_data_t;

int RR(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time);

int MFQ(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time);

int IORR(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time);

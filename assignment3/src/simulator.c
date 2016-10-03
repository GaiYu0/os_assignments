/**************************************/
/* (c) L. Apvrille, Telecom ParisTech */
/**************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scheduling.h>

#define MAX_EVENT_COUNT 256

char * states[] = {"upcoming", "ready", "running", "suspended", "terminated"};

char * state_string(int state) { return states[state]; }

int exising_unterminated_tasks(task_t tasks[], int task_count) {
    int total = 0;
    int i;
    
    for(i=0; i<task_count; i++) {
        if (tasks[i].state != TERMINATED) {
            total ++;
        }
    }
    return total;
}


void print_tasks(task_t tasks[], int task_count) {
    int i;
    
    for(i=0; i<task_count; i++) {
        printf("Task:%-4s\tArrival Date:%-4d\tState:%-10s\tComputations:%d/%d\n",
               tasks[i].name, tasks[i].arrival_date, state_string(tasks[i].state),
               tasks[i].execution_time, tasks[i].computation_time);
    }
}


typedef int (*scheduler_pointer)(
  task_t tasks[],
  int task_count,
  scheduler_data_t *scheduler_data,
  int current_time
);


scheduler_pointer parse_scheduler(char *name){
  if (strcmp(name, "FCFS") == 0)
    return &FCFS;
  else if (strcmp(name, "SJF") == 0)
    return &SJF;
  else if (strcmp(name, "SRTF") == 0)
    return &SRTF;
  else if (strcmp(name, "RR") == 0)
    return &RR;
  else if (strcmp(name, "MFQ") == 0)
    return &MFQ;
  else if (strcmp(name, "IORR") == 0)
    return &IORR;
  else
    return NULL;
}

int load_tasks(task_t tasks[], int max_task_count, FILE *file){
  char line [MAX_LINE_SIZE];
  int task_count = 0;
  while ((fgets(line, sizeof(line), file) != NULL) && (task_count != max_task_count)) {
    sscanf(
      line,
      "%s %u %u %u %u\n", 
      tasks[task_count].name,
      &(tasks[task_count].computation_time),
      &(tasks[task_count].arrival_date),
      &(tasks[task_count].io_duration),
      &(tasks[task_count].io_period)
    );
    tasks[task_count].state = UPCOMING;
    tasks[task_count].execution_time = 0;
    tasks[task_count].remaining_time = tasks[task_count].computation_time;
    tasks[task_count].waiting_time = 0;
    tasks[task_count].turnaround_time = 0;
    task_count ++;
  }
  return task_count;
}

int load_events(char *events[], int max_event_count, FILE *file){
  char line [MAX_LINE_SIZE];
  int event_count = 0;
  while ((fgets(line, sizeof(line), file) != NULL) && (event_count != max_event_count)) {
    if (line[0] != '#') { // ignore comments
      if (asprintf(&events[event_count], "%s", line) == -1) {
        // failure
        return -1;
      }
      event_count ++;
    }
  }
  return event_count;
}

int main(int argc, char *argv[]){
  scheduler_pointer scheduler;
  if (argc > 2) {
    scheduler = parse_scheduler(argv[2]);
    if (scheduler == NULL) {
      printf("Invalid scheduler!\n");
      return -1;
    }
  }

  task_t tasks[MAX_TASK_COUNT];
  scheduler_data_t *scheduler_data = (scheduler_data_t*)malloc(sizeof(scheduler_data_t));

  // quantum
  if (argc > 3) {
    scheduler_data->quantum = atoi(argv[3]);
  } else {
    scheduler_data->quantum = 0;
  }

  /* initialize schedule data */
  int i, j;
  for(i = 0; i != MAX_QUEUE_COUNT; i++){
    scheduler_data->starts[i] = 0;
    scheduler_data->ends[i] = 0;
    for(j = 0; j != MAX_TASK_COUNT; j++)
      scheduler_data->queues[i][j] = MAX_TASK_COUNT;
  }

  // load tasks
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
      perror(argv[1]);
      return -1;
  }
  int task_count = 0;
  task_count = load_tasks(tasks, MAX_TASK_COUNT, file);
  fclose(file);
  printf("%d tasks loaded\n", task_count);

  // load events
  int event_count = 0;
  char *events[MAX_EVENT_COUNT];
  if (argc > 4){
    file = fopen(argv[4], "r");
    if (file == NULL) {
        perror(argv[1]);
        return -1;
    }
    event_count = load_events(events, MAX_EVENT_COUNT, file);
    fclose(file);
  }
  printf("%d events loaded\n", event_count);

  // previous states
  int *previous_states = (int*)malloc(task_count);
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    previous_states[task_index] = UPCOMING;

  int event_index = 0;
  char *event_string;

  /**** Schedule the set of tasks ****/
  printf("Scheduling the set of tasks\n");
  
  int time;
  for(time = 0; exising_unterminated_tasks(tasks, task_count) > 0; time++) {
    // printf("%d\n", time);
    task_index = (*scheduler)(tasks, task_count, scheduler_data, time);
    /*
    printf("******************************\n");
    print_tasks(tasks, task_count);
    printf("******************************\n");
    if (task_index >= 0) {
        printf("\nTime %d: %s\n", time, tasks[task_index].name);
    } else {
        printf("\nTime %d: no task to schedule\n", time);
    }
    */

    for(task_index = 0; task_index != task_count; task_index++){
      // update statistics
      switch(tasks[task_index].state){
        case READY:
          tasks[task_index].waiting_time++;
          break;
        case TERMINATED:
          if(tasks[task_index].turnaround_time == 0){
            tasks[task_index].turnaround_time = time - tasks[task_index].arrival_date;
          }
          break;
      }
      if (tasks[task_index].state != previous_states[task_index]){
        // compare event generated by simulator to event in log
        asprintf(
          &event_string,
          "%d %s %s\n",
          time,
          tasks[task_index].name,
          state_string(tasks[task_index].state)
        );
        if (event_count != 0){
          printf("%s", event_string);
          printf("%s", events[event_index]);
          if ((event_index == event_count) || (strcmp(event_string, events[event_index]) != 0)) {
            printf("An unexpected event occurred.\n");
            return -1;
          }
          event_index ++;
        }
        previous_states[task_index] = tasks[task_index].state;
      }
    } // for
  } // for
  
  // print_tasks(tasks, task_count);
  printf("Finished in %d units of time\n", time);

  // metrics
  printf("###########\n");
  printf("# Metrics #\n");
  printf("###########\n");

  printf("|%-16s", "Task");
  printf("|%-16s", "Penalty rate");
  printf("|%-16s", "Turnaround time");
  printf("|%-16s", "Waiting time");
  printf("\n");

  float total_penalty_rate = 0;
  float total_turnaround_time = 0;
  float total_waiting_time = 0;

  for(task_index = 0; task_index != task_count; task_index++){
    // penalty rate
    float penalty_rate; 
    penalty_rate = (float)tasks[task_index].turnaround_time / tasks[task_index].computation_time;

    printf(
      "|%-16s|%-16f|%-16d|%-16d\n",
      tasks[task_index].name,
      penalty_rate,
      tasks[task_index].turnaround_time,
      tasks[task_index].waiting_time
    );

    // update total
    total_penalty_rate += penalty_rate;
    total_turnaround_time += tasks[task_index].turnaround_time;
    total_waiting_time += tasks[task_index].waiting_time;
  }
  printf(
    "|%-16s|%-16f|%-16f|%-16f\n",
    "Average",
    total_penalty_rate / task_count,
    total_turnaround_time / task_count,
    total_waiting_time / task_count
  );

  return 0;
}

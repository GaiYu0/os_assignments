#include <stdio.h>
#include <scheduling.h>

#define QUEUE_FCFS 1

void print_queues(task_t tasks[], scheduler_data_t* scheduler_data) {
    int i, j, task_index = 0;
    printf("Number of queues %d\n", scheduler_data->queue_count);
    for (i = 0; i < scheduler_data->queue_count; i++) {
        j = 0;
        printf("Q%d:", i);
        while (j < MAX_TASK_COUNT) {
            task_index = scheduler_data->queues[i][j];
            if (task_index == -1) {
                j = MAX_TASK_COUNT;
            } else {
                printf("%s ", tasks[task_index].name);
                j++;
            }
        }
        printf("\n");
    }
}


/*************************
* QUEUE PRIMITIVES START *
*************************/

int enqueue(scheduler_data_t* scheduler_data, int queue_index, int task_index) {
  int end;
  end = scheduler_data->ends[queue_index];
  if (scheduler_data->queues[queue_index][end] == MAX_TASK_COUNT) {
    scheduler_data->queues[queue_index][end] = task_index;
    scheduler_data->ends[queue_index] = (scheduler_data->ends[queue_index] + 1) % MAX_TASK_COUNT;
    return 0;
  } else {
    // memory exhausted
    return -1;
  }
}

int dequeue(scheduler_data_t* scheduler_data, int queue_index) {
  int start;
  start = scheduler_data->starts[queue_index];
  if (scheduler_data->queues[queue_index][start] == MAX_TASK_COUNT) {
    // empty queue
    return -1;
  } else {
    int task_index;
    task_index = scheduler_data->queues[queue_index][start];
    scheduler_data->queues[queue_index][start] = MAX_TASK_COUNT;
    scheduler_data->starts[queue_index] = (scheduler_data->starts[queue_index] + 1) % MAX_TASK_COUNT;
    return task_index;
  }
}

int front(scheduler_data_t* scheduler_data, int queue_index) {
  int start;
  start = scheduler_data->starts[queue_index];
  if (scheduler_data->queues[queue_index][start] == MAX_TASK_COUNT) {
    // empty queue
    return -1;
  } else {
    return scheduler_data->queues[queue_index][start];
  }
}

/***********************
* QUEUE PRIMITIVES END *
***********************/


#if QUEUE_FCFS
int FCFS(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    if((tasks[task_index].state == UPCOMING) && (tasks[task_index].arrival_date == current_time)){
      if(scheduler_data->queues[0][scheduler_data->ends[0]] == MAX_TASK_COUNT){
        scheduler_data->queues[0][scheduler_data->ends[0]] = task_index;
        scheduler_data->ends[0] = (scheduler_data->ends[0] + 1) % MAX_TASK_COUNT;
        tasks[task_index].state = READY;
      } else {
        // TODO
        return -1;
      }
    }

  if(scheduler_data->starts[0] != MAX_TASK_COUNT){
    int current_task_index;
    current_task_index = scheduler_data->queues[0][scheduler_data->starts[0]];
    tasks[current_task_index].execution_time++;
    if(tasks[current_task_index].execution_time == tasks[current_task_index].computation_time){
      tasks[current_task_index].state = TERMINATED;
      scheduler_data->starts[0] = (scheduler_data->starts[0] + 1) % MAX_TASK_COUNT;
      current_task_index = scheduler_data->queues[0][scheduler_data->starts[0]];
    }
    tasks[current_task_index].state = RUNNING;
    return current_task_index;
  }
  return -1;
}
#else
int FCFS(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
    
    int i, j;
    
    // Initialize single queue
    if (current_time == 0) {
        printf("Initializing job queue\n");
        scheduler_data->queue_count = 1;
        for (i = 0; i < MAX_TASK_COUNT; i++) {
            scheduler_data->queues[0][i] = -1;
        }
    }
    
    // Admit new tasks (current_time >= arrivalTime)
    j = 0;
    while (scheduler_data->queues[0][j] != -1)
        j++;
    for(i = 0; i < task_count; i++) {
        if ((tasks[i].state == UPCOMING) && (tasks[i].arrival_date == current_time)) {
            tasks[i].state = READY;
            scheduler_data->queues[0][j] = i;
            j++;
        }
    }
    print_queues(tasks, scheduler_data);
    
    // Is the first task in the queue running? Has that task finished its computations?
    //   If so, put it in terminated state and remove it from the queue
    //   If not, continue this task
    i = scheduler_data->queues[0][0];
    if (i != -1) {
        if (tasks[i].state == RUNNING) {
            if (tasks[i].execution_time == tasks[i].computation_time) {
                tasks[i].state = TERMINATED;
                for (j = 0; j < MAX_TASK_COUNT - 1; j++) {
                    scheduler_data->queues[0][j] = scheduler_data->queues[0][j+1];
                }
            } else {
                // Reelect this task
                tasks[i].execution_time ++;
                return i;
            }
        }
    }
    
    // Otherwise, elect the first task in the queue
    i = scheduler_data->queues[0][0];
    if (i != -1){
        tasks[i].execution_time ++;
        tasks[i].state = RUNNING;
        return i;
    }
    
    // No task could be elected
    return -1;
}
#endif


int SJF(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  // print_queues(tasks, scheduler_data);
  int current_task_index;
  current_task_index = scheduler_data->queues[0][scheduler_data->starts[0]];
  if(current_task_index != MAX_TASK_COUNT){
    tasks[current_task_index].execution_time++;
    if(tasks[current_task_index].execution_time == tasks[current_task_index].computation_time){
      tasks[current_task_index].state = TERMINATED;
      scheduler_data->starts[0] = (scheduler_data->starts[0] + 1) % MAX_TASK_COUNT;
    }
  }
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    if((tasks[task_index].state == UPCOMING) && (tasks[task_index].arrival_date == current_time)){
      if(scheduler_data->queues[0][scheduler_data->ends[0]] == MAX_TASK_COUNT){
        scheduler_data->queues[0][scheduler_data->ends[0]] = task_index;
        tasks[task_index].state = READY;
        scheduler_data->ends[0] = (scheduler_data->ends[0] + 1) % MAX_TASK_COUNT;
      } else {
        // TODO
        // memory exhausted
        return -1;
      }
      int queue_index;
      int next_index;
      // sort task queue
      if(scheduler_data->ends[0] - scheduler_data->starts[0] != 1)
        for(
          queue_index = (scheduler_data->ends[0] - 1) % MAX_TASK_COUNT;
          (next_index = (queue_index - 1) % MAX_TASK_COUNT) != scheduler_data->starts[0];
          queue_index = next_index
        )
          if(
            tasks[scheduler_data->queues[0][queue_index]].computation_time
              <
            tasks[scheduler_data->queues[0][next_index]].computation_time
          ){
            int cache;
            cache = scheduler_data->queues[0][queue_index];
            scheduler_data->queues[0][queue_index] = scheduler_data->queues[0][next_index];
            scheduler_data->queues[0][next_index] = cache;
          }
    }
  
  if(scheduler_data->queues[0][scheduler_data->starts[0]] != MAX_TASK_COUNT) {
    tasks[scheduler_data->queues[0][scheduler_data->starts[0]]].state = RUNNING;
    return scheduler_data->queues[0][scheduler_data->starts[0]];
  } else
    return -1;
}


int SRTF(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  // print_queues(tasks, scheduler_data);
  int current_task_index;
  current_task_index = scheduler_data->queues[0][scheduler_data->starts[0]];
  if(current_task_index != MAX_TASK_COUNT){
    tasks[current_task_index].execution_time++;
    tasks[current_task_index].remaining_time--;
    printf("%d %d\n", tasks[current_task_index].execution_time, tasks[current_task_index].computation_time);
    if(tasks[current_task_index].remaining_time == 0){
      tasks[current_task_index].state = TERMINATED;
      scheduler_data->starts[0] = (scheduler_data->starts[0] + 1) % MAX_TASK_COUNT;
    }
  }
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    if((tasks[task_index].state == UPCOMING) && (tasks[task_index].arrival_date == current_time)){
      if(scheduler_data->queues[0][scheduler_data->ends[0]] == MAX_TASK_COUNT){
        scheduler_data->queues[0][scheduler_data->ends[0]] = task_index;
        tasks[task_index].state = READY;
        scheduler_data->ends[0] = (scheduler_data->ends[0] + 1) % MAX_TASK_COUNT;
      } else {
        // TODO
        // memory exhausted
        return -1;
      }
      int queue_index;
      int next_index;
      // sort task queue
      if(scheduler_data->ends[0] - scheduler_data->starts[0] != 1)
        for(
          queue_index = (scheduler_data->ends[0] - 1) % MAX_TASK_COUNT;
          (next_index = (queue_index - 1) % MAX_TASK_COUNT) != scheduler_data->starts[0];
          queue_index = next_index
        )
          if(
            tasks[scheduler_data->queues[0][queue_index]].remaining_time
              <
            tasks[scheduler_data->queues[0][next_index]].remaining_time
          ){
            int cache;
            cache = scheduler_data->queues[0][queue_index];
            scheduler_data->queues[0][queue_index] = scheduler_data->queues[0][next_index];
            scheduler_data->queues[0][next_index] = cache;
          }
    }
  
  if (scheduler_data->queues[0][scheduler_data->starts[0]] != MAX_TASK_COUNT) {
    tasks[scheduler_data->queues[0][scheduler_data->starts[0]]].state = RUNNING;
    return scheduler_data->queues[0][scheduler_data->starts[0]];
  } else
    return -1;
}


int RR(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  int current_task_index;
  current_task_index = front(scheduler_data, 0);

  // process running task
  if(current_task_index != MAX_TASK_COUNT){ // running
    tasks[current_task_index].execution_time++;
    tasks[current_task_index].quantum --;
    if(tasks[current_task_index].execution_time == tasks[current_task_index].computation_time) {
      // terminated
      tasks[current_task_index].state = TERMINATED;
      dequeue(scheduler_data, 0);
    } else if (tasks[current_task_index].quantum == 0) {
      // the end of quantum
      tasks[current_task_index].state = READY;                     // reset state
      tasks[current_task_index].quantum = scheduler_data->quantum; // reset quantum
      dequeue(scheduler_data, 0);
      enqueue(scheduler_data, 0, current_task_index);
    }
  }

  // append tasks
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    if(tasks[task_index].arrival_date == current_time) {
      tasks[task_index].state = READY;                     // set state
      tasks[task_index].quantum = scheduler_data->quantum; // set quantum
      if (enqueue(scheduler_data, 0, task_index) == -1) {
        // memory exhausted
        return -1;
      }
    }

  // elect task
  current_task_index = front(scheduler_data, 0);
  if (current_task_index != MAX_TASK_COUNT) {
    tasks[current_task_index].state = RUNNING;
    return current_task_index;
  } else {
    return -1;
  }
}


#define PRIORITIES 3
int MFQ(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  if (current_time == 0) {
    scheduler_data->queue_count = PRIORITIES;
    scheduler_data->current_priority = 0;
  }

  int current_task_index;
  current_task_index = front(scheduler_data, scheduler_data->current_priority);
  if (tasks[current_task_index].state == RUNNING) {
    tasks[current_task_index].execution_time ++;
    tasks[current_task_index].quantum --;

    if (tasks[current_task_index].execution_time == tasks[current_task_index].computation_time) {
      // terminate
      tasks[current_task_index].state = TERMINATED;
      dequeue(scheduler_data, scheduler_data->current_priority);
    } else if (tasks[current_task_index].quantum == 0) {
      // the end of quantum
      tasks[current_task_index].state = READY;                    // reset state
      dequeue(scheduler_data, scheduler_data->current_priority);   // dequeue
      // modify priority
      int next_priority;
      next_priority = (scheduler_data->current_priority + 1) % PRIORITIES;
      tasks[current_task_index].quantum = (next_priority + 1) * scheduler_data->quantum; // quantum = P * Q
      enqueue(scheduler_data, next_priority, current_task_index);
    }
  }

  int task_index;
  for (task_index = 0; task_index != task_count; task_index++) {
    if (tasks[task_index].arrival_date == current_time) {
      // accept tasks
      tasks[task_index].state = READY;                    // reset state
      tasks[task_index].quantum = scheduler_data->quantum; // quantum = 1 * Q
      enqueue(scheduler_data, 0, task_index);
    }
  }

  // elect task
  // print_queues(tasks, scheduler_data);
  int queue_index;
  int front_task_index;
  for (
    queue_index = 0;
    queue_index != PRIORITIES;
    queue_index ++
  ) {
    front_task_index = front(scheduler_data, queue_index);
    if (front_task_index == -1) {
      // empty queue
      continue;
    }
    scheduler_data->current_priority = queue_index;
    tasks[front_task_index].state = RUNNING;
    return front_task_index;
  }
  // no task to elect
  return -1;
}


// Round Robin scheduler supporting IO interruption.
#define TASK_QUEUE 0
#define IO_QUEUE 1
int IORR(task_t tasks[], int task_count, scheduler_data_t* scheduler_data, int current_time) {
  int current_task_index;
  current_task_index = front(scheduler_data, TASK_QUEUE);

  // process running task
  if(current_task_index != MAX_TASK_COUNT){ // running
    tasks[current_task_index].execution_time++;
    tasks[current_task_index].quantum --;

    // io interruption
    if (tasks[current_task_index].io_period != 0) {
      if (tasks[current_task_index].execution_time % tasks[current_task_index].io_period == 0) {
        tasks[current_task_index].state = SLEEPING;                                          // set state
        tasks[current_task_index].sleeping_time = tasks[current_task_index].io_duration + 1; // set sleeping time
        tasks[current_task_index].quantum = scheduler_data->quantum;                         // set quantum
        dequeue(scheduler_data, TASK_QUEUE);
        enqueue(scheduler_data, IO_QUEUE, current_task_index);
      }
    }

    if (tasks[current_task_index].state == RUNNING) {
      if (tasks[current_task_index].execution_time == tasks[current_task_index].computation_time) {
        // terminated
        tasks[current_task_index].state = TERMINATED;
        dequeue(scheduler_data, 0);
      } else if (tasks[current_task_index].quantum == 0) {
        // the end of quantum
        tasks[current_task_index].state = READY;                     // reset state
        tasks[current_task_index].quantum = scheduler_data->quantum; // reset quantum
        dequeue(scheduler_data, 0);
        enqueue(scheduler_data, 0, current_task_index);
      }
    }
  }

  // append tasks
  int task_index;
  for(task_index = 0; task_index != task_count; task_index++)
    if(tasks[task_index].arrival_date == current_time) {
      tasks[task_index].state = READY;                     // set state
      tasks[task_index].quantum = scheduler_data->quantum; // set quantum
      if (enqueue(scheduler_data, 0, task_index) == -1) {
        // memory exhausted
        return -1;
      }
    }
  
  int io_queue_index;
  int io_task_index;
  // update sleeping time
  for (task_index = 0; task_index != task_count; task_index++) {
    if (tasks[task_index].state == SLEEPING) {
      tasks[task_index].sleeping_time --;
    }
  }
  if (current_time != 0) {
    for (
      io_queue_index = scheduler_data->starts[IO_QUEUE];
      io_queue_index != scheduler_data->ends[IO_QUEUE];
      io_queue_index = (io_queue_index + 1) % MAX_TASK_COUNT
    ) {
      io_task_index = scheduler_data->queues[IO_QUEUE][io_queue_index];

      // resume task
      if (tasks[io_task_index].sleeping_time == 0) {
        tasks[io_task_index].state = READY; // set state
        // move the task to the front of io queue
        while (io_queue_index != scheduler_data->starts[IO_QUEUE]) {
          int index_cache;
          index_cache = dequeue(scheduler_data, IO_QUEUE);
          enqueue(scheduler_data, IO_QUEUE, index_cache);
        }
        // remove the task from io queue
        dequeue(scheduler_data, IO_QUEUE);
        if (tasks[io_task_index].execution_time == tasks[io_task_index].computation_time) {
          // terminate
          printf("here\n");
          tasks[io_task_index].state = TERMINATED;
        } else {
          // to task queue
          enqueue(scheduler_data, TASK_QUEUE, io_task_index);
        }
      }
    }
  }

  // elect task
  current_task_index = front(scheduler_data, 0);
  if (current_task_index != MAX_TASK_COUNT) {
    tasks[current_task_index].state = RUNNING;
    return current_task_index;
  } else {
    return -1;
  }
}
